#pragma once

#include <array>

#include "common/ITypes.h"
#include "skse64/GameForms.h"
#include "skse64/GameObjects.h"

namespace vw
{
	using FormID = UInt32;

	using Activator = ::TESObjectACTI;
	using Armor = ::TESObjectARMO;
	using Container = ::TESObjectCONT;
	using Enchantment = ::EnchantmentItem;
	using Form = ::TESForm;
	using FormList = ::BGSListForm;
	using GlobalVariable = ::TESGlobal;
	using Keyword = ::BGSKeyword;
	using Light = ::TESObjectLIGH;
	using MagicEffect = ::EffectSetting;
	using Potion = ::AlchemyItem;
	using Quest = ::TESQuest;
	using Scroll = ::ScrollItem;
	using Spell = ::SpellItem;
	using Weapon = ::TESObjectWEAP;

	class EffectID
	{
		UInt64 m_id{ 0 };
	public:
		EffectID() {}
		constexpr EffectID(int i) : m_id{ 1Ui64 << i } {}

		EffectID& add(const EffectID& other) 
		{
			m_id |= other.m_id;
			return *this;
		}

		explicit operator bool() const { return m_id != 0; }
		explicit operator UInt64() const { return m_id; }

		friend constexpr bool operator==(const EffectID& lhs, const EffectID& rhs)
		{
			return lhs.m_id == rhs.m_id;
		}

		friend constexpr bool operator!=(const EffectID& lhs, const EffectID& rhs)
		{
			return !(lhs == rhs);
		}

		friend constexpr bool operator<(const EffectID& lhs, const EffectID& rhs)
		{
			return lhs.m_id < rhs.m_id;
		}

		friend constexpr EffectID operator|(const EffectID& lhs, const EffectID& rhs)
		{
			EffectID res;
			res.m_id = lhs.m_id | rhs.m_id;
			return res;
		}

		friend constexpr EffectID operator&(const EffectID& lhs, const EffectID& rhs)
		{
			EffectID res;
			res.m_id = lhs.m_id & rhs.m_id;
			return res;
		}

	public:
		static constexpr int size() { return 64; };

	};

	using hand_t = UInt32;
	constexpr hand_t HAND_LEFT{ 0 };
	constexpr hand_t HAND_RIGHT{ 1 };
	constexpr std::array<hand_t, 2> HANDS{ HAND_LEFT, HAND_RIGHT };
}
