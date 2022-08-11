#pragma once

#include "EffectLibrary.h"
#include "LightAdjuster.h"
#include "WeaponAnalyser.h"

constexpr vw::FormID EQUIP_ABILITY{ 0xa0a };

extern const ModInfo* g_modInfo;
extern vw::EffectLibrary g_effectLib;
extern vw::LightAdjuster g_lightAdjuster;
extern vw::WeaponAnalyser g_weaponAnalyser;