#include "pch.h"
#include "EffectLibrary.h"
#include "SerialisationUtil.h"
#include "version.h"

constexpr const char* ERROR_ID{ "EffectLibrary::Serialiser" };

namespace vw
{
	constexpr UInt32 FXLB = 'FXLB';
	constexpr UInt32 FXLB_currentVersion = 1;

	static FormList* readFormList(const SKSESerializationInterface& ssi)
	{
		FormID formID = readFormId(ssi, ERROR_ID);
		return formID ? DYNAMIC_CAST(LookupFormByID(formID), TESForm, BGSListForm) : nullptr;
	}


	bool EffectLibrary::Serialiser::read(const SKSESerializationInterface& ssi, EffectLibrary& lib)
	{
		//We don't need to lock anything here, no one will be touching the library during the load callback.
		UInt32 type, version, length;
		if (ssi.GetNextRecordInfo(&type, &version, &length)) {
			if (type == FXLB) {
				if (version == FXLB_currentVersion) {
					/*
					We expect to find:
					UInt32 nEffectLists
					FormID flags
					FormID... effectList
					*/
					UInt32 nEffectLists = 0;
					//UInt32 nExceptions = 0;
					if (!ssi.ReadRecordData(&nEffectLists, sizeof(UInt32)))
						_ERROR("%s error: %s", ERROR_ID, "Failed to read effect list number.");
					if (length == sizeof(UInt32) + (nEffectLists + 1) * sizeof(FormID)) {
						//Read the rest
						lib.setFlags(readFormList(ssi));//Should we always assign this?
						for (UInt32 i = 0; i < nEffectLists; i++) {
							lib.addEffects(readFormList(ssi));
							/*
							We never check if we managed to resolve the form id or not. We don't really care. 
							If it failed, we will just assume the form has been uninstalled and silently exclude 
							it from our data. What else could have happened?
							*/
						}
					}
					else
						_ERROR("%s error: %s", ERROR_ID, "Inconsistent data.");
				}
				else
					_ERROR("%s error: %s", ERROR_ID, "Unknown data version.");
			}
			else
				_ERROR("%s error: %s", ERROR_ID, "Unknown record type.");
		}
		else
			_ERROR("%s error: %s", ERROR_ID, "No data found.");

		return m_status;
	}

	bool EffectLibrary::Serialiser::write(const SKSESerializationInterface& ssi, const EffectLibrary& lib)
	{
		/*
		UInt32 nEffectLists
		FormID flags
		FormID... effect lists
		*/
		if (ssi.OpenRecord(FXLB, FXLB_currentVersion)) {
			std::shared_lock lock(lib.m_mutex);//we can keep the lock the whole time, no problem

			//We should write both the built and any pending lists (probably none, but it's possible).
			//Otherwise the pending lists would be lost.
			UInt32 nEffectsLists = lib.m_buildOrder.size() + lib.m_pendingOrder.size();
			if (!ssi.WriteRecordData(&nEffectsLists, sizeof(nEffectsLists)))
				_ERROR("%s error: %s", ERROR_ID, "Failed to write number of effect lists.");

			FormID flags = lib.m_flags ? lib.m_flags->formID : 0;
			if (!ssi.WriteRecordData(&flags, sizeof(flags)))
				_ERROR("%s error: %s", ERROR_ID, "Failed to write flags.");

			for (FormList* effect : lib.m_buildOrder) {
				assert(effect);
				if (!ssi.WriteRecordData(&effect->formID, sizeof(effect->formID)))
					_ERROR("%s error: %s", ERROR_ID, "Failed to write form ID.");
			}
			for (FormList* effect : lib.m_pendingOrder) {
				assert(effect);
				if (!ssi.WriteRecordData(&effect->formID, sizeof(effect->formID)))
					_ERROR("%s error: %s", ERROR_ID, "Failed to write form ID.");
			}
		}
		else
			_ERROR("%s error: %s", ERROR_ID, "Failed to open record.");

		return m_status;
	}
}