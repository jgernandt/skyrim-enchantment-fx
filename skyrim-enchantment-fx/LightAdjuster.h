#pragma once
#include <set>
#include "types.h"

struct SKSESerializationInterface;

namespace vw
{
	class LightAdjuster
		//Note: we are not thread-safing this class.
	{
	private:
		struct ScalingFactors
		{
			float fade{ 1.f };
			float radius{ 1.f };
			operator bool() const { return fade != 1.f || radius != 1.f; }
		};

		class Adjuster final :
			public FormList::Visitor
		{
		public:
			Adjuster(const ScalingFactors& factors) : m_factors{ factors } {}
			virtual bool Accept(Form* form) override;
		public:
			const ScalingFactors& m_factors;
		private:
			static std::set<FormID> s_adjustedLights;//in case a light is in two or more lists
		};

		class Serialiser final
		{
		public:
			Serialiser() {}
			bool read(const SKSESerializationInterface& ssi, LightAdjuster& lib);
			bool write(const SKSESerializationInterface& ssi, const LightAdjuster& lib);
		private:
			int m_status{ 1 };
		};

	public:
		LightAdjuster() {}
		void addLightList(FormList* lightList);

		void deserialise(const SKSESerializationInterface& ssi);
		void serialise(const SKSESerializationInterface& ssi) const;
		void print() const;
		void setReferences(Light* reference, Light* current);
	private:
		ScalingFactors m_factors;
		//These all go into the save:
		Light* m_refLight{ nullptr };
		Light* m_currentLight{ nullptr };
		std::set<FormID> m_lightLists;
	};
}

