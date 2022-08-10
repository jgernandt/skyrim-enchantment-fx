#include "pch.h"
#include "EffectLibrary.h"
#include "LogUtil.h"

vw::FormID vw::EffectLibrary::Builder::s_flag_LEFT{ 0 };

vw::EffectID vw::EffectLibrary::findSourceID(const MagicEffect& mgef) const
{
	std::shared_lock lock(m_mutex);
	if (auto it = m_data.effectIDs.find(mgef.formID); it != m_data.effectIDs.end())
		return it->second;
	else
		return EffectID();
}

vw::Spell* vw::EffectLibrary::findSideEffect(EffectID id, hand_t hand) const
{
	assert(hand < 2);

	Spell* spell = nullptr;
	if (id) {
		std::shared_lock lock(m_mutex);
		if (auto it = m_data.sideEffects.find(id); it != m_data.sideEffects.end())
			spell = it->second[hand] ? it->second[hand] : it->second[HAND_RIGHT];
	}
	return spell;
}

vw::EffectID vw::EffectLibrary::getId(FormID formID) const
{
	std::shared_lock lock(m_mutex);
	if (auto it = m_data.identifiers.find(formID); it != m_data.identifiers.end())
		return it->second;
	else
		return EffectID();
}

void vw::EffectLibrary::print() const
{
	_DMESSAGE("EffectLibrary");
	_DMESSAGE("  Flags:\t%08X", m_flags ? m_flags->formID : 0);
	_DMESSAGE("  Build order:");
	int i = 0;
	for (FormList* list : m_buildOrder)
		_DMESSAGE("    %02d %08X (%s)", i++, list->formID, getModNameByFormId(list->formID));
}

void vw::EffectLibrary::deserialise(const SKSESerializationInterface& ssi)
{
	Serialiser loader;
	if (!loader.read(ssi, *this))
		_ERROR("Failed to read data.");
	build();
	print();
}

void vw::EffectLibrary::serialise(const SKSESerializationInterface& ssi) const
{
	print();
	Serialiser saver;
	if (!saver.write(ssi, *this))
		_ERROR("Failed to write data.");
}

void vw::EffectLibrary::addEffects(FormList* effects)
{
	//Does not need sync with self. NoWait, called from a single script instance (or load callback).
	//Does not need sync with build, same reason.

	if (effects) {//Called from papyrus::update and serialiser::read. Neither checks for null.
		m_pendingOrder.push_back(effects);
	}
}

void vw::EffectLibrary::build()
{
	//We aren't really optimising for the most common case, that build order and pending order are equal.
	//Doesn't really matter though, this is an infrequent call.
	bool needBuild = false;
	for (FormList* pending : m_pendingOrder) {
		assert(pending);
		//Just insert in order in build order. We don't need pending list to be sorted.
		auto it = m_buildOrder.begin();
		while (it != m_buildOrder.end() && (*it)->formID < pending->formID)//Sort the list by form id
			++it;
		if (it == m_buildOrder.end() || (*it)->formID != pending->formID) {//ignore duplicates
			m_buildOrder.insert(it, pending);
			needBuild = true;
		}
	}
	m_pendingOrder.clear();

	if (needBuild) {
		/*_DMESSAGE("Build order:");
		i = 0;
		for (FormList* flst : m_buildOrder) {
			_DMESSAGE("  %02d %08X", i++, flst->formID);
		}*/

		_MESSAGE("Building effect library...");
		EffectData tmpData;
		Builder builder(tmpData);
		for (auto rit = m_buildOrder.rbegin(); rit != m_buildOrder.rend(); ++rit) {
			assert(*rit);//The only way into the list goes through addEffects, which tests for null
			_DMESSAGE("  Reading %08X (%s)...", (*rit)->formID, getModNameByFormId((*rit)->formID));
			(*rit)->Visit(builder);
		}

		//This needs sync with getSideEffects
		std::lock_guard lock(m_mutex);
		m_data = std::move(tmpData);

		_MESSAGE("Build completed:");
		_MESSAGE("  Effects:\t%d", m_data.effectIDs.size());
		_MESSAGE("  Side effects:\t%d", m_data.sideEffects.size());
		_MESSAGE("  Identifiers:\t%d", builder.m_idCount);
	}
}

void vw::EffectLibrary::setFlags(FormList* flst)
{
	//No sync needed between setFlags and loadCallback, obviously
	m_flags = flst;
	if (m_flags) {
		//LEFT should be first entry
		Form* flagLeft = nullptr;
		m_flags->forms.GetNthItem(0, flagLeft);
		Builder::s_flag_LEFT = flagLeft ? flagLeft->formID : 0;
	}
}
