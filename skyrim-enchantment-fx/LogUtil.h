#pragma once
#include "types.h"
#include "skse64/GameData.h"//DataHandler

namespace vw
{
	inline const char* getModNameByFormId(FormID formID)
	{
		auto modList = DataHandler::GetSingleton()->modList;
		UInt32 index = (formID & 0xFF000000) >> 24;
		if (index < 0xFE && index < modList.loadedMods.count && modList.loadedMods[index]) {
			return modList.loadedMods[index]->name;
		}
		else if (index == 0xFE) {
			UInt32 lightIndex = (formID & 0x00FFF000) >> 12;
			if (lightIndex < modList.loadedCCMods.count && modList.loadedCCMods[lightIndex])
				return modList.loadedCCMods[lightIndex]->name;
		}
		return "";
	}
}