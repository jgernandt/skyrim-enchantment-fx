#pragma once
#include <array>
#include <map>
#include <set>
#include <shared_mutex>
#include <stack>
#include "types.h"

struct SKSESerializationInterface;

namespace vw
{
	class EffectLibrary final
	{
	private:
		struct EffectData
		{
			EffectData() {}
			EffectData(const EffectData&) = delete;
			EffectData(EffectData&& other) noexcept { *this = std::move(other); }
			EffectData& operator=(const EffectData&) = delete;
			EffectData& operator=(EffectData&& other) noexcept
			{
				effectIDs = std::move(other.effectIDs);
				sideEffects = std::move(other.sideEffects);
				identifiers = std::move(other.identifiers);
				return *this;
			}
			std::map<FormID, EffectID> effectIDs;
			std::map<EffectID, std::array<Spell*, 2>> sideEffects;
			std::map<FormID, EffectID> identifiers;
		};

		class Builder final :
			public FormList::Visitor
		{
		private:
			struct State
			{
				EffectID id;
				hand_t hand{ HAND_RIGHT };
			};
		public:
			Builder(EffectData& data) : m_data{ data } { m_state.push({}); }
			virtual bool Accept(Form* form) override;
		private:
			bool process(FormList& flst);
			bool process(Keyword& kywd);
			bool process(MagicEffect& mgef);
			bool process(Spell& spel);
			bool process(Weapon& weap);
		public:
			static FormID s_flag_LEFT;
			int m_idCount{ 0 };
		private:
			EffectData& m_data;
			std::stack<State> m_state;
		};

		class Serialiser final
		{
		public:
			Serialiser() {}
			bool read(const SKSESerializationInterface& ssi, EffectLibrary& lib);
			bool write(const SKSESerializationInterface& ssi, const EffectLibrary& lib);
		private:
			int m_status{ 1 };
		};

	public:
		EffectLibrary() {}
		~EffectLibrary() {}

		EffectID findSourceID(const MagicEffect& mgef) const;
		Spell* findSideEffect(EffectID id, hand_t hand) const;
		EffectID getId(FormID formID) const;

		void print() const;

		void deserialise(const SKSESerializationInterface& ssi);
		void serialise(const SKSESerializationInterface& ssi) const;

		void addEffects(FormList* effects);
		void build();
		void setFlags(FormList* flst);

	private:
		void init();

	private:
		static Spell* s_equipAbility;

		mutable std::shared_mutex m_mutex;//we could have more than one if we need more fine-grained locking

		std::list<FormList*> m_buildOrder;
		std::list<FormList*> m_pendingOrder;

		EffectData m_data;
		FormList* m_flags{ nullptr };

	};
}

