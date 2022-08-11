#include "pch.h"

//Note: static initialisation must not be made dependent on game or skse version.

#include "EffectLibrary.h"
#include "LightAdjuster.h"
#include "WeaponAnalyser.h"

const ModInfo* g_modInfo{ nullptr };
vw::EffectLibrary g_effectLib;
vw::LightAdjuster g_lightAdjuster;
vw::WeaponAnalyser g_weaponAnalyser;