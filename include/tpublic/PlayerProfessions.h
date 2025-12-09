#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	class Manifest;

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

		void			ToStream(
							IWriter*					aWriter) const;
		bool			FromStream(
							IReader*					aReader);
		const Entry*	GetProfession(
							uint32_t					aProfessionId) const;
		Entry*			GetProfession(
							uint32_t					aProfessionId);
		bool			HasProfession(
							uint32_t					aProfessionId) const;
		bool			HasProfessionSkill(
							uint32_t					aProfessionId,
							uint32_t					aSkill) const;
		void			SetProfessionLevel(
							uint32_t					aProfessionId, 
							uint32_t					aProfessionLevel);
		void			AddProfessionAbility(
							uint32_t					aProfessionId,
							uint32_t					aAbilityId);
		bool			HasProfessionAbility(
							uint32_t					aProfessionId,
							uint32_t					aAbilityId) const;
		bool			RemoveProfession(
							uint32_t					aProfessionId);
		void			OnLoadedFromPersistence(
							const Manifest*				aManifest);
		bool			operator==(
							const PlayerProfessions&	aOther) const;
		bool			operator!=(
							const PlayerProfessions&	aOther) const;

		// Public data
		std::vector<Entry>	m_entries;
	};

}