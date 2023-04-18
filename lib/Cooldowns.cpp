#include "Pcheader.h"

#include <kpublic/Data/Ability.h>

#include <kpublic/Cooldowns.h>
#include <kpublic/Helpers.h>

namespace kpublic
{

	Cooldowns::Cooldowns()
	{

	}
	
	Cooldowns::~Cooldowns()
	{

	}

	void			
	Cooldowns::Update(
		uint32_t				aTick)
	{
		for(size_t i = 0; i < m_entries.size(); i++)
		{
			Entry& t = m_entries[i];

			if(t.m_end <= aTick)
			{
				Helpers::RemoveCyclicFromVector(m_entries, i);
				i--;
			}
		}
	}
	
	void			
	Cooldowns::Add(
		const Data::Ability*	aAbility,
		uint32_t				aTick)
	{
		uint32_t end = aTick + aAbility->m_cooldown;

		for (Entry& t : m_entries)
		{
			if(t.m_abilityId == aAbility->m_id)
			{
				if(end > t.m_end)
					t.m_end = end;

				return;
			}
		}

		m_entries.push_back({ aAbility->m_id, aTick, end });
	}
	
	const Cooldowns::Entry*
	Cooldowns::Get(
		uint32_t				aAbilityId) const
	{
		for (const Entry& t : m_entries)
		{
			if(t.m_abilityId == aAbilityId)
				return &t;
		}
		return NULL;
	}

}