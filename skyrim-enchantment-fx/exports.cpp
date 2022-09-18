#include "pch.h"

#include <ShlObj.h>  // CSIDL_MYDOCUMENTS

#include "skse64_common/skse_version.h"

#include "version.h"

constexpr UInt32 pluginID{ 'JGFX' };
constexpr const char* pluginName{ "Vibrant weapons" };
constexpr const char* logPath{ "\\My Games\\Skyrim Special Edition\\SKSE\\Vibrant weapons.log" };

namespace vw
{
	bool registerFunctions(VMClassRegistry* vcr);
	void loadCallback(SKSESerializationInterface* ssi);
	void saveCallback(SKSESerializationInterface* ssi);
}

extern "C" {

	__declspec(dllexport) SKSEPluginVersionData SKSEPlugin_Version = {
		SKSEPluginVersionData::kVersion,
		pluginVersion,
		"Vibrant weapons",
		"jgernandt",
		"",
		0,
		0,
		{ RUNTIME_VERSION_1_6_629, 0 },
		0,
	};

	__declspec(dllexport) bool SKSEPlugin_Load(const SKSEInterface* skse)
	{
		assert(skse);

		gLog.OpenRelative(CSIDL_MYDOCUMENTS, logPath);
#ifdef _DEBUG
		gLog.SetPrintLevel(IDebugLog::kLevel_DebugMessage);
		gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);
#else
		gLog.SetPrintLevel(IDebugLog::kLevel_VerboseMessage);
		gLog.SetLogLevel(IDebugLog::kLevel_VerboseMessage);
#endif

		UInt32 runtime = skse->runtimeVersion;
		SKSEPapyrusInterface* spi = static_cast<SKSEPapyrusInterface*>(skse->QueryInterface(kInterface_Papyrus));
		SKSESerializationInterface* ssi = static_cast<SKSESerializationInterface*>(skse->QueryInterface(kInterface_Serialization));

		_MESSAGE("Vibrant weapons.dll version %d.%d.%d", VW_VERSION_MAJOR, VW_VERSION_MINOR, VW_VERSION_PATCH);
		_MESSAGE("Runtime version %d.%d.%d",
			GET_EXE_VERSION_MAJOR(runtime),
			GET_EXE_VERSION_MINOR(runtime),
			GET_EXE_VERSION_BUILD(runtime));
		_MESSAGE("SKSE version %d.%d.%d\n",
			GET_EXE_VERSION_MAJOR(skse->skseVersion),
			GET_EXE_VERSION_MINOR(skse->skseVersion),
			GET_EXE_VERSION_BUILD(skse->skseVersion));

		if (!spi || !ssi) {
			_FATALERROR("Failed to get an SKSE interface. Plugin loading aborted.");
			return false;
		}

		_DMESSAGE("Loading plugin...");

		if (!spi->Register(vw::registerFunctions)) {
			_FATALERROR("Function registration failed. Plugin loading aborted.");
			return false;
		}

		PluginHandle ph = skse->GetPluginHandle();
		ssi->SetUniqueID(ph, pluginID);
		ssi->SetLoadCallback(ph, vw::loadCallback);
		ssi->SetSaveCallback(ph, vw::saveCallback);

		_MESSAGE("Plugin loaded successfully.\n");
		return true;
	}
};
