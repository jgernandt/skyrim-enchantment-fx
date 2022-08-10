#include "pch.h"
#include "SerialisationUtil.h"
#include "WeaponAnalyser.h"

constexpr const char* ERROR_ID{ "WeaponAnalyser::Serialiser" };

namespace vw
{
	constexpr UInt32 WPAN = 'WPAN';
	constexpr UInt32 WPAN_currentVersion = 1;

	static GlobalVariable* readGlobalVariable(const SKSESerializationInterface& ssi)
	{
		FormID formID = readFormId(ssi, ERROR_ID);
		return formID ? DYNAMIC_CAST(LookupFormByID(formID), TESForm, TESGlobal) : nullptr;
	}

	bool WeaponAnalyser::Serialiser::read(const SKSESerializationInterface& ssi, WeaponAnalyser& wpan)
	{
		//We don't need to lock anything here, no one will be touching the library during the load callback.
		UInt32 type, version, length;
		if (ssi.GetNextRecordInfo(&type, &version, &length)) {
			if (type == WPAN) {
				if (version == WPAN_currentVersion) {
					/*
					We expect to find:
					UInt32 nExceptions
					FormID lh global
					FormID rh global
					FormID... exceptions
					*/
					UInt32 nExceptions = 0;
					if (!ssi.ReadRecordData(&nExceptions, sizeof(UInt32)))
						_ERROR("%s error: %s", ERROR_ID, "Failed to read exception number.");
					//Now verify the length
					if (length == sizeof(UInt32) + (nExceptions + 2) * sizeof(FormID)) {
						//Read the rest
						GlobalVariable* left = readGlobalVariable(ssi);
						GlobalVariable* right = readGlobalVariable(ssi);
						wpan.setGlobals(left, right);
						for (UInt32 i = 0; i < nExceptions; i++)
							wpan.setException(readFormId(ssi, ERROR_ID));
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

	bool WeaponAnalyser::Serialiser::write(const SKSESerializationInterface& ssi, const WeaponAnalyser& wpan)
	{
		//nExceptions, 2 globals, exceptions
		if (ssi.OpenRecord(WPAN, WPAN_currentVersion)) {
			std::shared_lock lock(wpan.m_mutex);//we don't really need to lock, do we?

			UInt32 nExceptions = wpan.m_exceptions.size();
			if (!ssi.WriteRecordData(&nExceptions, sizeof(nExceptions)))
				_ERROR("%s error: %s", ERROR_ID, "Failed to write number of weapon exceptions.");

			for (hand_t hand : HANDS) {
				FormID formID = wpan.m_enchCost[hand] ? wpan.m_enchCost[hand]->formID : 0;
				if (!ssi.WriteRecordData(&formID, sizeof(formID)))
					_ERROR("%s error: %s", ERROR_ID, "Failed to write global.");
			}

			for (FormID formID : wpan.m_exceptions) {
				if (!ssi.WriteRecordData(&formID, sizeof(formID)))
					_ERROR("%s error: %s", ERROR_ID, "Failed to write form ID.");
			}
		}
		else
			_ERROR("%s error: %s", ERROR_ID, "Failed to open record.");

		return m_status;
	}
}
