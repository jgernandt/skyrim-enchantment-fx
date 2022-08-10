#include "pch.h"
#include "LightAdjuster.h"

std::set<vw::FormID> vw::LightAdjuster::Adjuster::s_adjustedLights;

void vw::LightAdjuster::addLightList(FormList* lightList)
{
	//No sync with setReferences required. They cannot run simultaneously.
	//No sync with ourselves required. We are called from the load callback and from addPlugin.
	//The latter is NoWait and called from the main script only.
	if (lightList && m_lightLists.insert(lightList->formID).second && m_factors)
		lightList->Visit(Adjuster(m_factors));
}

void vw::LightAdjuster::deserialise(const SKSESerializationInterface& ssi)
{
	Serialiser loader;
	loader.read(ssi, *this);
	print();
}

void vw::LightAdjuster::serialise(const SKSESerializationInterface& ssi) const
{
	print();
	Serialiser saver;
	saver.write(ssi, *this);
}

void vw::LightAdjuster::print() const
{
	_DMESSAGE("LightAdjuster");
	_DMESSAGE("  Reference:\t%08X", m_refLight ? m_refLight->formID : 0);
	_DMESSAGE("  Current:\t%08X", m_currentLight ? m_currentLight->formID : 0);
	_DMESSAGE("  Light lists:");
	for (FormID formID : m_lightLists)
		_DMESSAGE("    %08X", formID);
}

void vw::LightAdjuster::setReferences(Light* reference, Light* current)
{
	if (m_refLight != reference || m_currentLight != current) {
		m_refLight = reference;
		m_currentLight = current;
		m_factors.radius = (m_refLight && m_currentLight) ? static_cast<float>(m_currentLight->unkE0.radius) / m_refLight->unkE0.radius : 1.f;
		m_factors.fade = (m_refLight && m_currentLight) ? m_currentLight->fade / m_refLight->fade : 1.f;
	}
}

bool vw::LightAdjuster::Adjuster::Accept(Form* form)
{
	//We could allow flags in the light list as well, to specify what parameters are to be scaled, e.g.

	//Only adjust if insertion to m_adjustedLights succeeds
	if (form && form->formType == kFormType_Light && s_adjustedLights.insert(form->formID).second) {
		if (Light* light = DYNAMIC_CAST(form, TESForm, TESObjectLIGH)) {
			light->unkE0.radius *= m_factors.radius;
			light->fade *= m_factors.fade;
			_DMESSAGE("Adjusted %08X: radius %d, fade %.2f", light->formID, light->unkE0.radius, light->fade);
		}
	}
	return false;
}
