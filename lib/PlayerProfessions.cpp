#include "Pcheader.h"

#include <tpublic/Data/Ability.h>

#include <tpublic/Manifest.h>
#include <tpublic/PlayerProfessions.h>

namespace tpublic
{

	void
	PlayerProfessions::ToStream(
		IWriter*			aWriter) const
	{
		aWriter->WriteObjects(m_entries);
	}

	bool
	PlayerProfessions::FromStream(
		IReader*			aReader)
	{
		if (!aReader->ReadObjects(m_entries))
			return false;
		return true;
	}

	const PlayerProfessions::Entry*
	PlayerProfessions::GetProfession(
		uint32_t			aProfessionId) const
	{
		for(const Entry& t : m_entries)
		{
			if(t.m_professionId == aProfessionId)
				return &t;
		}
		return NULL;
	}

	PlayerProfessions::Entry*
	PlayerProfessions::GetProfession(
		uint32_t			aProfessionId) 
	{
		for(Entry& t : m_entries)
		{
			if(t.m_professionId == aProfessionId)
				return &t;
		}
		return NULL;
	}

	bool
	PlayerProfessions::HasProfession(
		uint32_t			aProfessionId) const
	{
		return GetProfession(aProfessionId) != NULL;
	}

	bool
	PlayerProfessions::HasProfessionSkill(
		uint32_t			aProfessionId,
		uint32_t			aSkill) const
	{
		const Entry* t = GetProfession(aProfessionId);
		if(t == NULL)
			return false;

		return t->m_skill >= aSkill;
	}

	void
	PlayerProfessions::SetProfessionLevel(
		uint32_t			aProfessionId, 
		uint32_t			aProfessionLevel)
	{
		for (Entry& t : m_entries)
		{
			if (t.m_professionId == aProfessionId)
			{
				t.m_professionLevel = aProfessionLevel;
				return;
			}
		}

		m_entries.push_back({ aProfessionId, aProfessionLevel, 0, {} });
	}

	void
	PlayerProfessions::AddProfessionAbility(
		uint32_t			aProfessionId,
		uint32_t			aAbilityId)
	{
		for (Entry& t : m_entries)
		{
			if (t.m_professionId == aProfessionId)
			{
				t.m_abilityIds.push_back(aAbilityId);
				return;
			}
		}
	}

	bool
	PlayerProfessions::HasProfessionAbility(
		uint32_t			aProfessionId,
		uint32_t			aAbilityId) const
	{
		for (const Entry& t : m_entries)
		{
			if (t.m_professionId == aProfessionId)
			{
				for(uint32_t abilityId : t.m_abilityIds)
				{
					if(abilityId == aAbilityId)
						return true;
				}
				break;
			}
		}

		return false;
	}

	bool
	PlayerProfessions::RemoveProfession(
		uint32_t			aProfessionId)
	{
		for(size_t i = 0; i < m_entries.size(); i++)
		{
			if(m_entries[i].m_professionId == aProfessionId)
			{
				Helpers::RemoveCyclicFromVector(m_entries, i);
				return true;
			}
		}
		return false;
	}

	void
	PlayerProfessions::OnLoadedFromPersistence(
		const Manifest*				aManifest)
	{
		// Remove invalid abilities (recipes that no longer exists)
		for (Entry& entry : m_entries)
		{
			for(uint32_t i = 0; i < entry.m_abilityIds.size(); i++)
			{
				if(aManifest->TryGetById<tpublic::Data::Ability>(entry.m_abilityIds[i]) == NULL)
				{
					entry.m_abilityIds.erase(entry.m_abilityIds.begin() + i);
					i--;
				}
			}
		}
	}

	bool
	PlayerProfessions::operator==(
		const PlayerProfessions&	aOther) const
	{
		if(m_entries.size() != aOther.m_entries.size())
			return false;
		for(size_t i = 0; i < m_entries.size(); i++)
		{
			if(m_entries[i] != aOther.m_entries[i])
				return false;
		}
		return true;
	}

	bool
	PlayerProfessions::operator!=(
		const PlayerProfessions&	aOther) const
	{
		return !this->operator==(aOther);
	}

}