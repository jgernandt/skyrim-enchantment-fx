#include "pch.h"
#include "EffectLibrary.h"
#include "LogUtil.h"

bool vw::EffectLibrary::Builder::Accept(Form* form)
{
	bool ret = false;
	if (form) {
		switch (form->formType) {
		case kFormType_EffectSetting:
			if (MagicEffect* mgef = DYNAMIC_CAST(form, TESForm, EffectSetting))
				ret = process(*mgef);
			break;
		case kFormType_Keyword:
			if (Keyword* kywd = DYNAMIC_CAST(form, TESForm, BGSKeyword))
				ret = process(*kywd);
			break;
		case kFormType_List:
			if (FormList* flst = DYNAMIC_CAST(form, TESForm, BGSListForm))
				ret = process(*flst);
			break;
		case kFormType_Spell:
			if (Spell* spel = DYNAMIC_CAST(form, TESForm, SpellItem))
				ret = process(*spel);
			break;
		case kFormType_Weapon:
			if (Weapon* weap = DYNAMIC_CAST(form, TESForm, TESObjectWEAP))
				ret = process(*weap);
			break;
		default:
			//Whatever they were trying to do, it's not supported. 
			//So whatever they were trying to do, it probably doesn't make sense. Ignore the rest of this branch.
			_WARNING("[WARNING] Unsupported entry of type %d found. Branch ignored.", form->formType);
			ret = true;
		}
	}

	return ret;
}

bool vw::EffectLibrary::Builder::process(FormList& flst)
{
	//Traverse this form list first, but don't keep any of its state changes.
	assert(!m_state.empty());

	m_state.push(m_state.top());
	flst.Visit(*this);
	m_state.pop();

	return false;
}

bool vw::EffectLibrary::Builder::process(Keyword& kywd)
{
	//Set the appropriate flag, or register a new identifier at this KYWDs form id and add it to our state id
	assert(!m_state.empty());
	State& ourState = m_state.top();

	bool ret = false;
	if (kywd.formID == s_flag_LEFT) {
		ourState.hand = HAND_LEFT;
	}
	else {
		//Enter in data.identifiers, add to our state
		if (m_idCount == EffectID::size()) {
			//We can register no more ids, but we can still search for it
			if (auto it = m_data.identifiers.find(kywd.formID); it != m_data.identifiers.end())
				ourState.id = ourState.id | it->second;
			else {
				_WARNING("[WARNING] Identifier limit (%d) exceeded. %s (and any associated effects) will be ignored.",
					EffectID::size(), kywd.keyword.Get());
				//We ignore the rest of this branch. It could result in all sorts of incorrect entries.
				ret = true;
			}
		}
		else {
			auto res = m_data.identifiers.insert({ kywd.formID, EffectID(m_idCount) });
			ourState.id = ourState.id | res.first->second;
			if (res.second) {
				m_idCount++;
				_DMESSAGE("    Registered identifier %s (id:%08X)", kywd.keyword.Get(), res.first->second);
			}
		}
	}
	return ret;
}

bool vw::EffectLibrary::Builder::process(MagicEffect& mgef)
{
	//Register our current state id at this MGEFs form id
	assert(!m_state.empty());
	State& ourState = m_state.top();

	if (ourState.id) {
		if (auto res = m_data.effectIDs.insert({ mgef.formID, ourState.id }); res.second) {
			_DMESSAGE("    Registered source effect %s (id:%08X)", mgef.fullName.GetName(), ourState.id);
			
			//Set our equip ability
			if (Spell* equipAb = mgef.properties.equipAbility) {
				//Append this mgef to the existing Spell's effect list, *unless* it's already there 
				//(can happen if we build multiple libs during a session!)
				bool ignore = false;
				for (UInt32 i = 0; i < equipAb->effectItemList.count; i++) {
					if (equipAb->effectItemList[i] && equipAb->effectItemList[i]->mgef == &mgef) {
						ignore = true;
						break;
					}
				}
				if (!ignore) {
					//We never clean this up. It lasts for the duration of the game session, so doesn't really matter.
					//If nothing else, destruction of g_mainHeap will free it.
					MagicItem::EffectItem* item = new MagicItem::EffectItem;
					*item = *EffectLibrary::s_equipAbility->effectItemList[0];
					equipAb->effectItemList.Push(item);
				}
			}
			else {
				mgef.properties.equipAbility = EffectLibrary::s_equipAbility;
			}
		}
		else {
			//Insertion failed. Should we combine the ids? I don't think so, the results could be unpredictable.
		}
	}
	else
		_WARNING("[WARNING] %s has no identifiers. Entry ignored.", mgef.fullName.GetName());
	
	return false;
}

bool vw::EffectLibrary::Builder::process(Spell& spel)
{
	//Register this SPEL at our current state id
	assert(!m_state.empty());
	State& ourState = m_state.top();

	if (ourState.id) {
		auto res = m_data.sideEffects.insert({ ourState.id, { nullptr, nullptr } });
		if (res.second)
			_DMESSAGE("    Registered side effect %s (id:%08X)", spel.fullName.GetName(), ourState.id);
		if (!res.first->second[ourState.hand])
			res.first->second[ourState.hand] = &spel;
	}
	else
		_WARNING("[WARNING] %s has no identifiers. Entry ignored.", spel.fullName.GetName());
	
	return false;
}

bool vw::EffectLibrary::Builder::process(Weapon& weap)
{
	//Register our current state id at this WEAPs form id
	//We could use a 'UNIQUE' flag if we wanted to register the form ID as an identifier.
	assert(!m_state.empty());
	State& ourState = m_state.top();

	if (ourState.id) {
		if (auto res = m_data.identifiers.insert({ weap.formID, ourState.id }); res.second) {
			_DMESSAGE("    Registered extra identifier for %s (id:%08X)", weap.fullName.GetName(), res.first->second);
		}
		else {
			//Weapon was added already, what do we do? Ignore, assuming that a later-loaded plugin did it?
		}
	}
	else
		_WARNING("[WARNING] %s has no identifiers. Entry ignored.", weap.fullName.GetName());
	
	return false;
}
