#include "pch.h"
#include "SerialisationUtil.h"
#include "LightAdjuster.h"

constexpr const char* ERROR_ID{ "LightAdjuster::Serialiser" };

namespace vw
{
	constexpr UInt32 LADJ = 'LADJ';
	constexpr UInt32 LADJ_currentVersion = 1;

	static Light* readLight(const SKSESerializationInterface& ssi)
	{
		FormID formID = readFormId(ssi, ERROR_ID);
		return formID ? DYNAMIC_CAST(LookupFormByID(formID), TESForm, TESObjectLIGH) : nullptr;
	}

	static FormList* readFormList(const SKSESerializationInterface& ssi)
	{
		FormID formID = readFormId(ssi, ERROR_ID);
		return formID ? DYNAMIC_CAST(LookupFormByID(formID), TESForm, BGSListForm) : nullptr;
	}

	bool LightAdjuster::Serialiser::read(const SKSESerializationInterface& ssi, LightAdjuster& ladj)
	{
		//We don't need to lock anything here, no one will be touching the library during the load callback.
		UInt32 type, version, length;
		if (ssi.GetNextRecordInfo(&type, &version, &length)) {
			if (type == LADJ) {
				if (version == LADJ_currentVersion) {
					/*
					We expect to find:
					UInt32 nLightLists
					FormID ref light
					FormID cur light
					FormID light lists
					...
					*/
					UInt32 nLightLists = 0;
					if (!ssi.ReadRecordData(&nLightLists, sizeof(UInt32)))
						_ERROR("%s error: %s", ERROR_ID, "Failed to read light list number.");
					//Now verify the length
					if (length == sizeof(UInt32) + (nLightLists + 2) * sizeof(FormID)) {
						//Read the rest
						Light* reference = readLight(ssi);
						Light* current = readLight(ssi);
						ladj.setReferences(reference, current);
						for (UInt32 i = 0; i < nLightLists; i++)
							ladj.addLightList(readFormList(ssi));//We could save a lookup if its already adjusted
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

	bool LightAdjuster::Serialiser::write(const SKSESerializationInterface& ssi, const LightAdjuster& ladj)
	{
		//nLightLists, 2 Lights, lightLists
		if (ssi.OpenRecord(LADJ, LADJ_currentVersion)) {

			UInt32 nLightLists = ladj.m_lightLists.size();
			if (!ssi.WriteRecordData(&nLightLists, sizeof(nLightLists)))
				_ERROR("%s error: %s", ERROR_ID, "Failed to write number of light lists.");

			FormID refLightID = ladj.m_refLight ? ladj.m_refLight->formID : 0;
			if (!ssi.WriteRecordData(&refLightID, sizeof(refLightID)))
				_ERROR("%s error: %s", ERROR_ID, "Failed to write reference light.");

			FormID curLightID = ladj.m_currentLight ? ladj.m_currentLight->formID : 0;
			if (!ssi.WriteRecordData(&curLightID, sizeof(curLightID)))
				_ERROR("%s error: %s", ERROR_ID, "Failed to write current light.");

			for (FormID formID : ladj.m_lightLists) {
				if (!ssi.WriteRecordData(&formID, sizeof(formID)))
					_ERROR("%s error: %s", ERROR_ID, "Failed to write light list.");
			}
		}
		else
			_ERROR("%s error: %s", ERROR_ID, "Failed to open record.");

		return m_status;
	}
}
