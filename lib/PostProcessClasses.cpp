#include "Pcheader.h"

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/Class.h>
#include <tpublic/Data/TalentTree.h>

#include <tpublic/Manifest.h>

#include "PostProcessClasses.h"

namespace tpublic
{

	namespace PostProcessClasses
	{

		void
		Run(
			Manifest* aManifest)
		{
			// Figure out which class each talent tree belongs to
			aManifest->GetContainer<Data::Class>()->ForEach([aManifest](
				Data::Class* aClass)
			{
				for(uint32_t talentTreeId : aClass->m_talentTrees)
				{
					Data::TalentTree* talentTree = aManifest->GetById<Data::TalentTree>(talentTreeId);
					TP_VERIFY(talentTree->m_classId == 0 || talentTree->m_classId == aClass->m_id, aClass->m_debugInfo, "Talent tree '%s' belongs to more than one class.", talentTree->m_name.c_str());
					talentTree->m_classId = aClass->m_id;
				}
				return true;
			});

			// Verify hybrid talent trees
			aManifest->GetContainer<Data::Class>()->ForEach([aManifest](
				Data::Class* aClass)
			{
				// Only resources and abilities are used from a 'hybrid' level progression
				if(aClass->m_hybrid && aClass->m_hybrid->m_levelProgression)
				{
					for(const std::unique_ptr<Data::Class::LevelProgressionLevel>& level : aClass->m_hybrid->m_levelProgression->m_levels)
					{
						TP_VERIFY(level->m_stats.IsEmpty() && level->m_addUnarmedWeaponDamageMin == 0 && level->m_addUnarmedWeaponDamageMax == 0 && level->m_achievementId == 0,
							aClass->m_debugInfo, "Hybrid level progression (level %u) can only have resources and abilities.", level->m_level);
					}
				}

				std::unordered_set<uint32_t> hybridTalentTreeIds;

				for(const Data::Class::HybridTalentTree& hybridTalentTree : aClass->m_hybridTalentTrees)
				{
					const Data::TalentTree* talentTree = aManifest->GetById<Data::TalentTree>(hybridTalentTree.m_talentTreeId);

					const std::optional<DataErrorHandling::DebugInfo>& debugInfo = hybridTalentTree.m_debugInfo;

					TP_VERIFY(!hybridTalentTreeIds.contains(talentTree->m_id), debugInfo, "Hybrid talent tree '%s' listed more than once.", talentTree->m_name.c_str());
					hybridTalentTreeIds.insert(talentTree->m_id);

					TP_VERIFY(talentTree->m_classId != 0, debugInfo, "Hybrid talent tree '%s' doesn't belong to any class.", talentTree->m_name.c_str());
					TP_VERIFY(talentTree->m_classId != aClass->m_id, debugInfo, "Hybrid talent tree '%s' belongs to the class itself.", talentTree->m_name.c_str());

					const Data::Class* donorClass = aManifest->GetById<Data::Class>(talentTree->m_classId);
					TP_VERIFY(donorClass->m_hybrid, debugInfo, "Hybrid talent tree '%s' belongs to '%s', which has no 'hybrid' definition.", talentTree->m_name.c_str(), donorClass->m_name.c_str());

					// Extra level progression can only unlock abilities
					if(hybridTalentTree.m_levelProgression)
					{
						for(const std::unique_ptr<Data::Class::LevelProgressionLevel>& level : hybridTalentTree.m_levelProgression->m_levels)
						{
							TP_VERIFY(level->m_stats.IsEmpty() && level->m_resourceUpdates.empty() && level->m_addUnarmedWeaponDamageMin == 0 && level->m_addUnarmedWeaponDamageMax == 0 && level->m_achievementId == 0,
								debugInfo, "Hybrid talent tree '%s' level progression (level %u) can only have abilities.", talentTree->m_name.c_str(), level->m_level);
						}
					}

					// Excluded abilities must be ones the hybrid would otherwise get from the donor class
					for(uint32_t excludedAbilityId : hybridTalentTree.m_excludeAbilities)
					{
						bool found = false;

						for(const std::unique_ptr<Data::Class::LevelProgressionLevel>& level : donorClass->m_levelProgression->m_levels)
						{
							for(uint32_t abilityId : level->m_unlockAbilities)
							{
								if(abilityId == excludedAbilityId && aManifest->GetById<Data::Ability>(abilityId)->m_talentTreeId == talentTree->m_id)
									found = true;
							}
						}

						if(donorClass->m_hybrid->m_levelProgression)
						{
							for(const std::unique_ptr<Data::Class::LevelProgressionLevel>& level : donorClass->m_hybrid->m_levelProgression->m_levels)
							{
								for(uint32_t abilityId : level->m_unlockAbilities)
								{
									if(abilityId == excludedAbilityId)
										found = true;
								}
							}
						}

						TP_VERIFY(found, debugInfo, "Hybrid talent tree '%s' excludes '%s', which it wouldn't get anyway.", talentTree->m_name.c_str(), aManifest->GetById<Data::Ability>(excludedAbilityId)->m_name.c_str());
					}
				}
				return true;
			});
		}

	}

}
