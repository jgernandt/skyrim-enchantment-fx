#include "pch.h"
#include "types.h"
#include "WeaponAnalyser.h"

namespace referenceUtils
{
	//defined in skse64/PapyrusWornObject.cpp
	EquipData ResolveEquippedObject(Actor* actor, UInt32 weaponSlot, UInt32 slotMask);
}

namespace vw
{
	static EffectID getEnchID(const Enchantment& e, const EffectLibrary& lib)
	{
		//For each mgef in e, search and combine

		EffectID id;

		//tArray::operator[] has no const overload. This is a workaround for that. We are not changing the array.
		auto& ncList = const_cast<tArray<MagicItem::EffectItem*>&>(e.effectItemList);

		for (UInt32 i = 0; i < ncList.count; i++) {
			if (ncList[i] && ncList[i]->mgef) {
				id.add(lib.findSourceID(*ncList[i]->mgef));
			}
		}
		return id;
	}

	static EffectID getWeapID(const Weapon& w, const EffectLibrary& lib)
	{
		EffectID id = lib.getId(w.formID);
		for (UInt32 i = 0; i < w.keyword.numKeywords; i++) {
			if (w.keyword.keywords[i])
				id.add(lib.getId(w.keyword.keywords[i]->formID));
		}
		return id;
	}

	static EffectID getCostliestID(Enchantment* e, const EffectLibrary& lib)
	{
		assert(e);
		MagicItem::EffectItem* costliest = CALL_MEMBER_FN(e, GetCostliestEffectItem)(5, false);//from skse64/PapyrusSpell.cpp, don't know what the params are
		if (costliest && costliest->mgef) {
			return lib.findSourceID(*costliest->mgef);
		}
		else {
			return EffectID();
		}
	}

	WeaponAnalyser::result_t WeaponAnalyser::analyse(Weapon* weapon, Character* actor, hand_t hand, bool player, const EffectLibrary& lib)
	{
		assert(hand < 2);
		result_t res;

		EquipData equipData = referenceUtils::ResolveEquippedObject(actor, hand, 0);
		res.matchingWeapon = equipData.pForm == weapon;
		if (res.matchingWeapon && weapon && !isExcepted(weapon->templateForm ? *weapon->templateForm : *weapon)) {

			//Premade
			Enchantment* ench = weapon->enchantable.enchantment;
			//Player made
			if (!ench) {
				if (BaseExtraList* extraData = equipData.pExtraData) {
					if (ExtraEnchantment* extraEnch = static_cast<ExtraEnchantment*>(extraData->GetByType(kExtraData_Enchantment)))
						ench = extraEnch->enchant;
				}
			}

			if (ench) {
				//Search for exact match first, then costliest effect only
				EffectID weapID = getWeapID(*weapon, lib);
				res.sideEffect = lib.findSideEffect(getEnchID(*ench, lib) | weapID, hand);

				//We'll end up searching for the costliest effect sourceID twice, we could optimise it a bit
				if (!res.sideEffect)
					res.sideEffect = lib.findSideEffect(getCostliestID(ench, lib) | weapID, hand);

				//If this is the player, set the global cost variables
				if (player) {
					std::lock_guard lock(m_mutex);//sync with setGlobals. There is a (small) chance they could run simultaneously.
					if (res.sideEffect && m_enchCost[hand]) {

						//unk34 is a float (regardless of the type of the global)
						*reinterpret_cast<float*>(&m_enchCost[hand]->unk34) = CALL_MEMBER_FN(ench, GetEffectiveMagickaCost)(actor);
					}
				}
			}
		}
		return res;
	}

	void WeaponAnalyser::setGlobals(GlobalVariable* left, GlobalVariable* right)
	{
		std::lock_guard lock(m_mutex);//sync with analyse. There is a (small) chance they could run simultaneously.
		m_enchCost[HAND_LEFT] = left;
		m_enchCost[HAND_RIGHT] = right;
	}

	void WeaponAnalyser::deserialise(const SKSESerializationInterface& ssi)
	{
		Serialiser loader;
		loader.read(ssi, *this);
		print();
	}

	void WeaponAnalyser::serialise(const SKSESerializationInterface& ssi) const
	{
		print();
		Serialiser saver;
		saver.write(ssi, *this);
	}

	void WeaponAnalyser::setException(FormID formID, bool except)
	{
		std::lock_guard lock(m_mutex);
		if (except) 
			m_exceptions.insert(formID);
		else 
			m_exceptions.erase(formID);
	}

	bool WeaponAnalyser::isExcepted(const Weapon& weap) const
	{
		std::shared_lock lock(m_mutex);
		return m_exceptions.find(weap.formID) != m_exceptions.end();
	}

	void WeaponAnalyser::print() const
	{
		_DMESSAGE("WeaponAnalyser");
		_DMESSAGE("  LH object: %08X", m_enchCost[HAND_LEFT] ? m_enchCost[HAND_LEFT]->formID : 0);
		_DMESSAGE("  RH object: %08X", m_enchCost[HAND_RIGHT] ? m_enchCost[HAND_RIGHT]->formID : 0);
		_DMESSAGE("  Exceptions:");
		for (FormID formID : m_exceptions)
			_DMESSAGE("    %08X", formID);
	}

}