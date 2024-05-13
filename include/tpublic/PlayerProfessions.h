#pragma once

#include "Helpers.h"
#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	class PlayerProfessions
	{
	public:
		struct Entry
		{
			void
			ToStream(
				IWriter*		aWriter) const
			{
				aWriter->WriteUInt(m_professionId);
				aWriter->WriteUInt(m_professionLevel);
				aWriter->WriteUInt(m_skill);
				aWriter->WriteUInts(m_abilityIds);
			}

			bool
			FromStream(
				IReader*		aReader)
			{
				if (!aReader->ReadUInt(m_professionId))
					return false;
				if (!aReader->ReadUInt(m_professionLevel))
					return false;
				if (!aReader->ReadUInt(m_skill))
					return false;
				if (!aReader->ReadUInts(m_abilityIds))
					return false;
				return true;
			}

			bool
			operator==(
				const Entry&	aOther) const
			{
				return m_professionId == aOther.m_professionId && m_professionLevel == aOther.m_professionLevel && m_skill == aOther.m_skill && m_abilityIds == aOther.m_abilityIds;
			}

			bool
			operator!=(
				const Entry&	aOther) const
			{
				return !this->operator==(aOther);
			}

			bool
			HasAbility(
				uint32_t		aAbilityId) const
			{
				for(uint32_t t : m_abilityIds)
				{
					if(t == aAbilityId)
						return true;
				}
				return false;
			}

			// Public data
			uint32_t				m_professionId = 0;
			uint32_t				m_professionLevel = 0;
			uint32_t				m_skill = 0;
			std::vector<uint32_t>	m_abilityIds;
		};

		void
		ToStream(
			IWriter*			aWriter) const
		{
			aWriter->WriteObjects(m_entries);
		}

		bool
		FromStream(
			IReader*			aReader)
		{
			if (!aReader->ReadObjects(m_entries))
				return false;
			return true;
		}

		const Entry*
		GetProfession(
			uint32_t			aProfessionId) const
		{
			for(const Entry& t : m_entries)
			{
				if(t.m_professionId == aProfessionId)
					return &t;
			}
			return NULL;
		}

		Entry*
		GetProfession(
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
		HasProfession(
			uint32_t			aProfessionId) const
		{
			return GetProfession(aProfessionId) != NULL;
		}

		bool
		HasProfessionSkill(
			uint32_t			aProfessionId,
			uint32_t			aSkill) const
		{
			const Entry* t = GetProfession(aProfessionId);
			if(t == NULL)
				return false;

			return t->m_skill >= aSkill;
		}

		void
		SetProfessionLevel(
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
		AddProfessionAbility(
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
		RemoveProfession(
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

		bool
		operator==(
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
		operator!=(
			const PlayerProfessions&	aOther) const
		{
			return !this->operator==(aOther);
		}

		// Public data
		std::vector<Entry>	m_entries;
	};

}