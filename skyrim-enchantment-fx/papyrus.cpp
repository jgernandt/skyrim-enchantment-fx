#include "pch.h"

#include "globals.h"
#include "Timer.h"
#include "types.h"

namespace referenceUtils
{
	//defined in skse64/PapyrusWornObject.cpp
	EquipData ResolveEquippedObject(Actor* actor, UInt32 weaponSlot, UInt32 slotMask);
}

namespace vw
{

	static bool addPlugin(StaticFunctionTag*, FormList* effects, FormList* lights)
	{
		try {
			g_effectLib.addEffects(effects);
			g_lightAdjuster.addLightList(lights);
			return true;
		}
		catch (const std::exception& e) {
			_ERROR(e.what());
			return false;
		}
	}

	static bool getSideEffects(StaticFunctionTag*, Weapon* w, Character* actor, VMArray<Spell*> r_spells, VMArray<bool> r_bools, bool player)
	{
		try {
			Timer<long, std::micro> timer;

			if (r_spells.Length() == 2 && r_bools.Length() == 2) {
				for (auto hand : HANDS) {
					if (auto res = g_weaponAnalyser.analyse(w, actor, hand, player, g_effectLib); res.matchingWeapon) {
						r_bools.Set(&res.matchingWeapon, hand);
						r_spells.Set(&res.sideEffect, hand);

						_DMESSAGE("  %s hand update: %s",
							hand == HAND_LEFT ? "Left" : "Right",
							res.sideEffect ? res.sideEffect->fullName.GetName() : "no effect");
					}

				}
			}
			_DMESSAGE("Finishing getSideEffects in %d µs.", timer.elapsed());
			return true;
		}
		catch (const std::exception& e) {
			_ERROR(e.what());
			return false;
		}
	}

	static bool initialise(StaticFunctionTag*, VMArray<FormList*> flsts, VMArray<GlobalVariable*> globs, VMArray<Light*> lighs)
	{
		try {
			if (flsts.Length() != 1 || globs.Length() != 2 || lighs.Length() != 2)
				_WARNING("[WARNING] Unexpected initialisation parameters. Library may not function as intended.");

			if (flsts.Length() > 0) {
				FormList* flst;
				flsts.Get(&flst, 0);
				g_effectLib.setFlags(flst);
			}
			if (globs.Length() > 1) {
				GlobalVariable* left;
				GlobalVariable* right;
				globs.Get(&left, 0);
				globs.Get(&right, 1);
				g_weaponAnalyser.setGlobals(left, right);
			}
			if (lighs.Length() > 1) {
				Light* reference;
				Light* current;
				lighs.Get(&reference, 0);
				lighs.Get(&current, 1);
				g_lightAdjuster.setReferences(reference, current);
			}
			return true;
		}
		catch (const std::exception& e) {
			_ERROR(e.what());
			return false;
		}
	}


	//Save for later
	/*constexpr UInt32 chargeAVs[]{ 82, 64 };//put somewhere safe

	static bool isEquippedWeaponCharged(StaticFunctionTag*, Character* actor, UInt32 hand)
	{
		Timer<long, std::micro> timer;
		bool ret = false;
		if (actor && hand < 2) {
			float charge = actor->actorValueOwner.GetCurrent(chargeAVs[hand]);
			if (charge > 0.f) {
				EquipData equipData = resolveEquippedObject(actor, hand, 0);
				if (equipData.pForm) {
					TESEnchantableForm* enchantable = DYNAMIC_CAST(equipData.pForm, TESForm, TESEnchantableForm);
					Enchantment* ench = enchantable ? enchantable->enchantment : nullptr;
					if (!ench && equipData.pExtraData) {
						ExtraEnchantment* extraEnch = static_cast<ExtraEnchantment*>(equipData.pExtraData->GetByType(kExtraData_Enchantment));
						ench = extraEnch ? extraEnch->enchant : nullptr;
					}

					if (ench)
						ret = CALL_MEMBER_FN(ench, GetEffectiveMagickaCost)(actor) <= charge;
				}
			}
		}
		_DMESSAGE("Finishing isEquippedWeaponCharged in %d µs.", timer.elapsed());
		return ret;
	}*/

	static bool isWeaponDetached(StaticFunctionTag*, Actor* actor, BSFixedString nodeName)
	{
		bool ret = true;
		if (actor) {
			NiAVObject* object = actor->GetNiNode();
			if (nodeName.data[0] && object)
				object = object->GetObjectByName(&nodeName.data);

			NiNode* node = ni_cast(object, NiNode);
			if (node && node->m_children.m_size) {
				//The node exists and has something attached
				if (node->m_children.m_data[0] && node->m_children.m_data[0]->m_name && node->m_children.m_data[0]->m_name[0] == 'W') {
					//The name of the first child begins with W, so is probably a weapon (there's really no way to make this absolutely failsafe, is there?)
					ret = false;
				}
			}

		}
		return ret;
	}

	static bool setException(StaticFunctionTag*, Actor* actor, bool except)
	{
		try {
			if (actor) {
				for (auto hand : HANDS) {
					if (EquipData equipData = referenceUtils::ResolveEquippedObject(actor, hand, 0); equipData.pForm) {
						if (Weapon* weap = DYNAMIC_CAST(equipData.pForm, TESForm, TESObjectWEAP)) {
							//if templated, set the base form instead
							g_weaponAnalyser.setException(weap->templateForm ? weap->templateForm->formID : weap->formID, except);
						}
					}
				}
			}
			return true;
		}
		catch (const std::exception& e) {
			_ERROR(e.what());
			return false;
		}
	}

	static bool updateLibrary(StaticFunctionTag*)
	{
		try {
			Timer<long, std::micro> timer;

			g_effectLib.build();

			_DMESSAGE("Finishing updateLibrary in %d µs.", timer.elapsed());
			return true;
		}
		catch (const std::exception& e) {
			_ERROR(e.what());
			return false;
		}
	}

	bool registerFunctions(VMClassRegistry* vcr)
	{
		assert(vcr);

		vcr->RegisterFunction(new NativeFunction2<StaticFunctionTag,
			bool,
			FormList*, FormList*>
			("AddPlugin", "JGFX_Native", &addPlugin, vcr));

		vcr->RegisterFunction(new NativeFunction5<StaticFunctionTag,
			bool,
			Weapon*, Character*, VMArray<Spell*>, VMArray<bool>, bool>
			("GetSideEffects", "JGFX_Native", &getSideEffects, vcr));

		vcr->RegisterFunction(new NativeFunction3<StaticFunctionTag,
			bool,
			VMArray<FormList*>, VMArray<GlobalVariable*>, VMArray<Light*>>
			("Initialise", "JGFX_Native", &initialise, vcr));

		/*vcr->RegisterFunction(new NativeFunction2<StaticFunctionTag,
			bool,
			Character*, UInt32>
			("IsEquippedWeaponCharged", "JGFX_Native", &isEquippedWeaponCharged, vcr));*/

		vcr->RegisterFunction(new NativeFunction2<StaticFunctionTag,
			bool,
			Actor*, BSFixedString>
			("IsWeaponDetached", "JGFX_Native", &isWeaponDetached, vcr));

		vcr->RegisterFunction(new NativeFunction2<StaticFunctionTag,
			bool,
			Actor*, bool>
			("SetException", "JGFX_Native", &setException, vcr));

		vcr->RegisterFunction(new NativeFunction0<StaticFunctionTag,
			bool
			/*void*/>
			("UpdateLibrary", "JGFX_Native", &updateLibrary, vcr));

		vcr->SetFunctionFlags("JGFX_Native", "AddPlugin", VMClassRegistry::kFunctionFlag_NoWait);

		return true;
	}

}