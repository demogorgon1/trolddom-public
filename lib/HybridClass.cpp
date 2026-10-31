#include "Pcheader.h"

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/TalentTree.h>

#include <tpublic/HybridClass.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	namespace HybridClass
	{

		namespace
		{

			bool
			_Contains(
				const std::vector<uint32_t>&					aIds,
				uint32_t										aId)
			{
				for(uint32_t id : aIds)
				{
					if(id == aId)
						return true;
				}
				return false;
			}

			void
			_AddUnique(
				std::vector<uint32_t>&							aIds,
				uint32_t										aId)
			{
				if(!_Contains(aIds, aId))
					aIds.push_back(aId);
			}

		}

		//------------------------------------------------------------------------------------------

		const Data::Class*
		GetDonorClass(
			const Manifest*										aManifest,
			uint32_t											aHybridTalentTreeId)
		{
			if(aHybridTalentTreeId == 0)
				return NULL;

			const Data::TalentTree* talentTree = aManifest->TryGetById<Data::TalentTree>(aHybridTalentTreeId);
			if(talentTree == NULL || talentTree->m_classId == 0)
				return NULL;

			const Data::Class* donorClass = aManifest->TryGetById<Data::Class>(talentTree->m_classId);
			if(donorClass == NULL || !donorClass->m_hybrid)
				return NULL;

			return donorClass;
		}

		bool
		IsValid(
			const Manifest*										aManifest,
			const Data::Class*									aClass,
			const Components::Talents*							aTalents)
		{
			if(!aTalents->IsHybrid())
				return true;

			return aClass->HasTalentTree(aTalents->m_hybridReplacedTalentTreeId) &&
				aClass->GetHybridTalentTree(aTalents->m_hybridTalentTreeId) != NULL &&
				GetDonorClass(aManifest, aTalents->m_hybridTalentTreeId) != NULL;
		}

		bool
		CanUseItemType(
			const Manifest*										aManifest,
			const Data::Class*									aClass,
			uint32_t											aHybridTalentTreeId,
			ItemType::Id										aItemType)
		{
			if(aClass->CanUseItemType(aItemType))
				return true;

			if(aHybridTalentTreeId != 0 && GetDonorClass(aManifest, aHybridTalentTreeId) != NULL)
			{
				const Data::Class::HybridTalentTree* hybridTalentTree = aClass->GetHybridTalentTree(aHybridTalentTreeId);
				if(hybridTalentTree != NULL)
					return (hybridTalentTree->m_itemTypesMask & (1 << (uint32_t)aItemType)) != 0;
			}

			return false;
		}

		void
		GetSystems(
			const Manifest*										aManifest,
			const Data::Class*									aClass,
			uint32_t											aHybridTalentTreeId,
			std::vector<uint32_t>&								aOut)
		{
			aOut = aClass->m_systems;

			const Data::Class* donorClass = GetDonorClass(aManifest, aHybridTalentTreeId);
			if(donorClass != NULL)
			{
				for(uint32_t systemId : donorClass->m_hybrid->m_systems)
					_AddUnique(aOut, systemId);
			}
		}

		void
		GetStatsConversion(
			const Manifest*										aManifest,
			const Data::Class*									aClass,
			uint32_t											aHybridTalentTreeId,
			Data::Class::StatsConversion&						aOut)
		{
			aOut = aClass->m_statsConversion;

			const Data::Class* donorClass = GetDonorClass(aManifest, aHybridTalentTreeId);
			const Data::Class::HybridTalentTree* hybridTalentTree = aClass->GetHybridTalentTree(aHybridTalentTreeId);
			if(donorClass != NULL && hybridTalentTree != NULL)
			{
				aOut.CombineMax(donorClass->m_hybrid->m_statsConversion);
				aOut.CombineMax(hybridTalentTree->m_statsConversion);
			}
		}

		void
		GetLevelProgression(
			const Manifest*										aManifest,
			const Data::Class*									aClass,
			const Components::Talents*							aTalents,
			uint32_t											aLevel,
			LevelProgression&									aOut)
		{
			aOut.m_levels.clear();
			aOut.m_hybridLevels.clear();
			aOut.m_abilityIds.clear();

			// Core abilities: a class keeps these even if it has replaced the talent tree they belong to
			std::vector<uint32_t> coreAbilityIds;
			if(aClass->m_hybrid && aClass->m_hybrid->m_levelProgression)
			{
				for(const std::unique_ptr<Data::Class::LevelProgressionLevel>& level : aClass->m_hybrid->m_levelProgression->m_levels)
				{
					if(aLevel >= level->m_level)
					{
						for(uint32_t abilityId : level->m_unlockAbilities)
							coreAbilityIds.push_back(abilityId);
					}
				}
			}

			// Class level progression, without abilities from a replaced talent tree
			for(const std::unique_ptr<Data::Class::LevelProgressionLevel>& level : aClass->m_levelProgression->m_levels)
			{
				if(aLevel < level->m_level)
					continue;

				aOut.m_levels.push_back(level.get());

				for(uint32_t abilityId : level->m_unlockAbilities)
				{
					const Data::Ability* ability = aManifest->GetById<Data::Ability>(abilityId);

					if(ability->m_talentTreeId == 0 || aTalents->CanUseTalentTree(aClass->m_talentTrees, ability->m_talentTreeId) || _Contains(coreAbilityIds, abilityId))
						_AddUnique(aOut.m_abilityIds, abilityId);
				}
			}

			if(!aTalents->IsHybrid())
				return;

			const Data::Class* donorClass = GetDonorClass(aManifest, aTalents->m_hybridTalentTreeId);
			const Data::Class::HybridTalentTree* hybridTalentTree = aClass->GetHybridTalentTree(aTalents->m_hybridTalentTreeId);
			if(donorClass == NULL || hybridTalentTree == NULL)
				return;

			// Donor class level progression abilities from the hybrid talent tree
			for(const std::unique_ptr<Data::Class::LevelProgressionLevel>& level : donorClass->m_levelProgression->m_levels)
			{
				if(aLevel < level->m_level)
					continue;

				for(uint32_t abilityId : level->m_unlockAbilities)
				{
					const Data::Ability* ability = aManifest->GetById<Data::Ability>(abilityId);

					if(ability->m_talentTreeId == aTalents->m_hybridTalentTreeId && !_Contains(hybridTalentTree->m_excludeAbilities, abilityId))
						_AddUnique(aOut.m_abilityIds, abilityId);
				}
			}

			// Donor class 'hybrid' level progression: resources and core abilities
			if(donorClass->m_hybrid->m_levelProgression)
			{
				for(const std::unique_ptr<Data::Class::LevelProgressionLevel>& level : donorClass->m_hybrid->m_levelProgression->m_levels)
				{
					if(aLevel < level->m_level)
						continue;

					aOut.m_hybridLevels.push_back(level.get());

					for(uint32_t abilityId : level->m_unlockAbilities)
					{
						if(!_Contains(hybridTalentTree->m_excludeAbilities, abilityId))
							_AddUnique(aOut.m_abilityIds, abilityId);
					}
				}
			}

			// Extra abilities for this particular combination
			if(hybridTalentTree->m_levelProgression)
			{
				for(const std::unique_ptr<Data::Class::LevelProgressionLevel>& level : hybridTalentTree->m_levelProgression->m_levels)
				{
					if(aLevel < level->m_level)
						continue;

					for(uint32_t abilityId : level->m_unlockAbilities)
						_AddUnique(aOut.m_abilityIds, abilityId);
				}
			}
		}

	}

}
