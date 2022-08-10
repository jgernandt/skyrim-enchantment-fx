#pragma once
#include "types.h"

namespace vw
{
	inline FormID readFormId(const SKSESerializationInterface& ssi, const char* source)
	{
		FormID ret = 0;
		FormID readID;
		if (ssi.ReadRecordData(&readID, sizeof(FormID))) {
			ssi.ResolveFormId(readID, &ret);
		}
		else {
			_ERROR("%s error: %s", source, "Failed to read form ID.");
		}
		return ret;
	}
}