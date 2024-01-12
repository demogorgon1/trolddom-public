#include "Pcheader.h"

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/Cooldown.h>

#include <tpublic/Cooldowns.h>
#include <tpublic/Helpers.h>
#include <tpublic/IReader.h>
#include <tpublic/IWriter.h>
#include <tpublic/Manifest.h>

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
		int32_t					aTick)
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
	Cooldowns::AddAbility(
		const Manifest*			aManifest,
		const Data::Ability*	aAbility,
		int32_t					aTick)
	{
		for(uint32_t cooldownId : aAbility->m_cooldowns)
		{
			const Data::Cooldown* cooldown = aManifest->GetById<Data::Cooldown>(cooldownId);

			int32_t end = aTick + cooldown->m_duration;
			bool updated = false;

			for (Entry& t : m_entries)
			{
				if (t.m_cooldownId == cooldownId)
				{
					if (end > t.m_end)
						t.m_end = end;

					updated = true;
					break;
				}
			}

			if(!updated)
				m_entries.push_back({ cooldownId, aTick, end });
		}
	}
	
	//const Cooldowns::Entry*
	//Cooldowns::Get(
	//	uint32_t				aAbilityId) const
	//{
	//	for (const Entry& t : m_entries)
	//	{
	//		if(t.m_abilityId == aAbilityId)
	//			return &t;
	//	}
	//	return NULL;
	//}

	bool			
	Cooldowns::IsAbilityOnCooldown(
		const Data::Ability*	aAbility) const
	{
		for (const Entry& t : m_entries)
		{
			for(uint32_t cooldownId : aAbility->m_cooldowns)
			{
				if(cooldownId == t.m_cooldownId)
					return true;
			}
		}
		return false;
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