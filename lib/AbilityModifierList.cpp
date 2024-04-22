#include "Pcheader.h"

#include <tpublic/Data/AbilityModifier.h>

#include <tpublic/AbilityModifierList.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	bool
	AbilityModifierList::SetAbilityModifierIds(
		const Manifest*					aManifest,
		const std::vector<uint32_t>&	aAbilityModifierIds)
	{
		if(aAbilityModifierIds == m_abilityModifierIds)
			return false;

		m_abilityModifierIds = aAbilityModifierIds;

		for(AbilityTable::iterator i = m_abilityTable.begin(); i != m_abilityTable.end(); i++)
			i->second->m_modifiers.clear();

		for(uint32_t abilityModifierId : aAbilityModifierIds)
		{
			const Data::AbilityModifier* abilityModifier = aManifest->GetById<Data::AbilityModifier>(abilityModifierId);

			AbilityTable::iterator i = m_abilityTable.find(abilityModifier->m_abilityId);
			Ability* entry = NULL;
			if(i != m_abilityTable.end())
			{
				entry = i->second.get();
			}
			else
			{
				entry = new Ability();
				m_abilityTable[abilityModifier->m_abilityId] = std::unique_ptr<Ability>(entry);
			}

			entry->m_modifiers.push_back(abilityModifier);
		}

		for (AbilityTable::iterator i = m_abilityTable.begin(); i != m_abilityTable.end();)
		{
			if(i->second->m_modifiers.empty())
				m_abilityTable.erase(i++);
			else
				i++;
		}

		return true;
	}
	
	void		
	AbilityModifierList::GetAbilityModifierIds(
		std::vector<uint32_t>&			aOut) const
	{
		aOut = m_abilityModifierIds;
	}

	const std::vector<const Data::AbilityModifier*>*
	AbilityModifierList::GetAbility(
		uint32_t						aAbilityId) const
	{
		AbilityTable::const_iterator i = m_abilityTable.find(aAbilityId);
		if(i == m_abilityTable.cend())
			return NULL;
		return &i->second->m_modifiers;
	}

	int32_t														
	AbilityModifierList::GetAbilityModifyAuraUpdateCount(
		uint32_t						aAbilityId) const
	{
		int32_t result = 0;
		const std::vector<const Data::AbilityModifier*>* abilityModifiers = GetAbility(aAbilityId);
		if(abilityModifiers != NULL)
		{
			for(const Data::AbilityModifier* abilityModifier : *abilityModifiers)
				result += abilityModifier->m_modifyAuraUpdateCount;
		}
		return result;
	}

	int32_t														
	AbilityModifierList::GetAbilityModifyRange(
		uint32_t						aAbilityId) const
	{
		int32_t result = 0;
		const std::vector<const Data::AbilityModifier*>* abilityModifiers = GetAbility(aAbilityId);
		if (abilityModifiers != NULL)
		{
			for (const Data::AbilityModifier* abilityModifier : *abilityModifiers)
				result += abilityModifier->m_modifyRange;
		}
		return result;
	}

}