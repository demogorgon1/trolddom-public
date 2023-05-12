#include "Pcheader.h"

#include <tpublic/Data/Ability.h>

#include <tpublic/Cooldowns.h>
#include <tpublic/Helpers.h>
#include <tpublic/IReader.h>
#include <tpublic/IWriter.h>

namespace tpublic
{

	Cooldowns::Cooldowns()
	{

	}
	
	Cooldowns::~Cooldowns()
	{

	}

	bool
	Cooldowns::Update(
		uint32_t				aTick)
	{
		bool changed = false;

		for(size_t i = 0; i < m_entries.size(); i++)
		{
			Entry& t = m_entries[i];

			if(t.m_end <= aTick)
			{
				Helpers::RemoveCyclicFromVector(m_entries, i);
				i--;

				changed = true;
			}
		}

		return changed;
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

	void			
	Cooldowns::ToStream(
		IWriter*				aStream) const
	{
		aStream->WriteObjects(m_entries);
	}
	
	bool			
	Cooldowns::FromStream(
		IReader*				aStream)
	{
		if(!aStream->ReadObjects(m_entries))
			return false;
		return true;
	}

}