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
		int32_t					aTick,
		float					aCooldownModifier)
	{
		for(uint32_t cooldownId : aAbility->m_cooldowns)
		{
			const Data::Cooldown* cooldown = aManifest->GetById<Data::Cooldown>(cooldownId);

			if(cooldown->m_trigger == Data::Cooldown::TRIGGER_ABILITY)
			{
				if(aCooldownModifier == 0.0f)
				{
					AddCooldown(cooldownId, cooldown->m_duration, aTick);
				}
				else
				{
					int32_t duration = (int32_t)((1.0f - (aCooldownModifier / 100.0f)) * (float)cooldown->m_duration);
					AddCooldown(cooldownId, duration, aTick);
				}
			}
		}
	}
	
	void			
	Cooldowns::AddCooldown(
		uint32_t				aCooldownId,
		int32_t					aDuration,
		int32_t					aTick)
	{
		int32_t end = aTick + aDuration;
		bool updated = false;

		for (Entry& t : m_entries)
		{
			if (t.m_cooldownId == aCooldownId)
			{
				if (end > t.m_end)
					t.m_end = end;

				updated = true;
				break;
			}
		}

		if (!updated)
			m_entries.push_back({ aCooldownId, aTick, end });
	}

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

	const Cooldowns::Entry*
	Cooldowns::GetCooldown(
		uint32_t				aCooldownId) const
	{
		for (const Entry& t : m_entries)
		{
			if(t.m_cooldownId == aCooldownId)
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