#include "pch.h"
#include "globals.h"
#include "Timer.h"
#include "version.h"

namespace vw
{
	constexpr UInt32 HEAD = 'HEAD';
	constexpr UInt32 HEAD_currentVersion = 1;

	static UInt32 readFileVersion(const SKSESerializationInterface& ssi)
	{
		UInt32 ret = 0;

		UInt32 type = 0, version, length;

		//We should loop until we find the header. There are situations where we will find records that aren't ours.
		//(specifically, if another skse plugin with saved data was uninstalled at the same time as ours was installed).
		//Did we mess something up? Or is this an skse bug?
		while (ssi.GetNextRecordInfo(&type, &version, &length) && type != HEAD) {
			_DMESSAGE("Found unexpected record of type %08X while looking for file header.", type);
		}

		if (type == HEAD) {
			if (version == 1) {
				if (!ssi.ReadRecordData(&ret, length))
					_ERROR("[ERROR] Failed to read file header.");
			}
			else
				_ERROR("[ERROR] Unknown file version.");
		}
		else
			_DMESSAGE("Found end of file while looking for file header.");

		return ret;
	}

	void loadCallback(SKSESerializationInterface* ssi)
	{
#ifdef _DEBUG
		Timer<long, std::micro> timer;
#endif
		if (ssi) {
			_DMESSAGE("Loading game...");

			UInt32 fileVersion = readFileVersion(*ssi);
			if (fileVersion) {
				g_weaponAnalyser.deserialise(*ssi);
				g_effectLib.deserialise(*ssi);
				g_lightAdjuster.deserialise(*ssi);
			}
		}
#ifdef _DEBUG
		_DMESSAGE("Finishing loadCallback in %d µs.", timer.elapsed());
#endif
	}

	void saveCallback(SKSESerializationInterface* ssi)
	{
#ifdef _DEBUG
		Timer<long, std::micro> timer;
#endif
		if (ssi) {
			if (ssi->WriteRecord(HEAD, HEAD_currentVersion, &pluginVersion, sizeof(pluginVersion))) {
				g_weaponAnalyser.serialise(*ssi);
				g_effectLib.serialise(*ssi);
				g_lightAdjuster.serialise(*ssi);
			}
			else
				_ERROR("[ERROR] Failed to write header. No data will be saved.");
		}
#ifdef _DEBUG
		_DMESSAGE("Finishing saveCallback in %d µs.", timer.elapsed());
#endif
	}

}