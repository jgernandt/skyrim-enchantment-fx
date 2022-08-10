#pragma once
#include "EffectLibrary.h"

namespace vw
{
	class WeaponAnalyser final
	{
	public:

		class Serialiser final
		{
		public:
			Serialiser() {}
			bool read(const SKSESerializationInterface& ssi, WeaponAnalyser& lib);
			bool write(const SKSESerializationInterface& ssi, const WeaponAnalyser& lib);
		private:
			int m_status{ 1 };
		};

		struct result_t
		{
			bool matchingWeapon{ false };
			Spell* sideEffect{ nullptr };
			//float cost{ 0.f };
		};

	public:
		WeaponAnalyser() {}
		~WeaponAnalyser() {}

		result_t analyse(Weapon* weapon, Character* actor, hand_t hand, bool player, const EffectLibrary& lib);
		void setGlobals(GlobalVariable* left, GlobalVariable* right);
		void deserialise(const SKSESerializationInterface& ssi);
		void serialise(const SKSESerializationInterface& ssi) const;

		void setException(FormID formID, bool except = true);
		bool isExcepted(const Weapon& weap) const;

		void print() const;
	private:
		mutable std::shared_mutex m_mutex;
		//save:
		GlobalVariable* m_enchCost[2]{ nullptr, nullptr };
		std::set<FormID> m_exceptions;
	};

}

