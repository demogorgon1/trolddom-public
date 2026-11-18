#include "Pcheader.h"

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/Achievement.h>
#include <tpublic/Data/Aura.h>
#include <tpublic/Data/Class.h>
#include <tpublic/Data/TalentTree.h>

#include <tpublic/ItemMetrics.h>
#include <tpublic/Manifest.h>
#include <tpublic/ToolTipMultiplier.h>

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

					// The gateway only knows about front end achievements
					const Data::Achievement* requiredAchievement = aManifest->GetById<Data::Achievement>(hybridTalentTree.m_requiredAchievementId);
					TP_VERIFY(requiredAchievement->m_availableInFrontEnd, debugInfo, "Hybrid talent tree '%s' requires achievement '%s', which isn't 'available_in_front_end'.", talentTree->m_name.c_str(), requiredAchievement->m_name.c_str());

					// Extra item types are shown with an icon from item metrics (for example on the talent calculator web page)
					for(uint32_t i = 1; i < (uint32_t)ItemType::NUM_IDS; i++)
					{
						if((hybridTalentTree.m_itemTypesMask & (1 << i)) != 0 && !aClass->CanUseItemType((ItemType::Id)i))
						{
							TP_VERIFY(aManifest->m_itemMetrics->m_itemTypeIconSpriteIds[i] != 0, debugInfo, "Hybrid talent tree '%s' can use '%s', which has no icon in 'item_type_icons'.",
								talentTree->m_name.c_str(), ItemType::GetInfo((ItemType::Id)i)->m_name);
						}
					}

					// Extra level progression can only unlock abilities and give achievements
					if(hybridTalentTree.m_levelProgression)
					{
						for(const std::unique_ptr<Data::Class::LevelProgressionLevel>& level : hybridTalentTree.m_levelProgression->m_levels)
						{
							TP_VERIFY(level->m_stats.IsEmpty() && level->m_resourceUpdates.empty() && level->m_addUnarmedWeaponDamageMin == 0 && level->m_addUnarmedWeaponDamageMax == 0,
								debugInfo, "Hybrid talent tree '%s' level progression (level %u) can only have abilities and achievements.", talentTree->m_name.c_str(), level->m_level);
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

					// Passive auras
					{
						// Abilities the hybrid has no matter which talent tree was replaced: the class' core abilities and the
						// donor class' 'hybrid' abilities
						std::unordered_set<uint32_t> alwaysAvailableAbilityIds;
						for(const Data::Class* t : { (const Data::Class*)aClass, donorClass })
						{
							if(t->m_hybrid && t->m_hybrid->m_levelProgression)
							{
								for(const std::unique_ptr<Data::Class::LevelProgressionLevel>& level : t->m_hybrid->m_levelProgression->m_levels)
								{
									for(uint32_t abilityId : level->m_unlockAbilities)
										alwaysAvailableAbilityIds.insert(abilityId);
								}
							}
						}

						std::unordered_set<uint32_t> passiveAuraIds;

						for(const Data::Class::HybridPassiveAura& passiveAura : hybridTalentTree.m_passiveAuras)
						{
							const Data::Aura* aura = aManifest->GetById<Data::Aura>(passiveAura.m_auraId);
							const std::optional<DataErrorHandling::DebugInfo>& auraDebugInfo = passiveAura.m_debugInfo;

							TP_VERIFY(!passiveAuraIds.contains(aura->m_id), auraDebugInfo, "Passive aura '%s' listed more than once.", aura->m_name.c_str());
							passiveAuraIds.insert(aura->m_id);

							// Characters have these auras permanently, the flag makes sure they're removed again when they no longer qualify
							TP_VERIFY(aura->m_hybridPassive, auraDebugInfo, "Passive aura '%s' must have 'hybrid_passive: true'.", aura->m_name.c_str());
							TP_VERIFY(aura->m_type == Data::Aura::TYPE_HIDDEN && (aura->m_flags & Data::Aura::FLAG_PERSIST_IN_DEATH) != 0 && aura->m_duration == 0,
								auraDebugInfo, "Passive aura '%s' must be 'type: hidden', 'persist_in_death' and have no duration.", aura->m_name.c_str());
							TP_VERIFY(aura->m_iconSpriteId != 0 && !aura->m_string.empty() && !aura->m_description.empty(), auraDebugInfo, "Passive aura '%s' needs an icon, a string and a description (it's shown in the abilities window).", aura->m_name.c_str());

							for(uint32_t requiredTalentTreeId : passiveAura.m_requiredTalentTreeIds)
							{
								TP_VERIFY(aClass->HasTalentTree(requiredTalentTreeId), auraDebugInfo, "Passive aura '%s' requires talent tree '%s', which isn't one of the class' own.",
									aura->m_name.c_str(), aManifest->GetById<Data::TalentTree>(requiredTalentTreeId)->m_name.c_str());
							}

							// Abilities the aura affects (as far as the tool tip multipliers tell) must be ones the character has: the
							// talent trees of any class abilities must be required, as they might have been replaced
							for(const std::unique_ptr<Data::Aura::AuraEffectEntry>& auraEffect : aura->m_auraEffects)
							{
								auraEffect->m_auraEffectBase->ForEachToolTipMultiplier([&](
									ToolTipMultiplier& aToolTipMultiplier)
								{
									for(uint32_t abilityId : aToolTipMultiplier.m_abilityIds)
									{
										const Data::Ability* ability = aManifest->GetById<Data::Ability>(abilityId);
										if(ability->m_talentTreeId == 0 || ability->m_talentTreeId == talentTree->m_id || alwaysAvailableAbilityIds.contains(abilityId))
											continue;

										TP_VERIFY(aClass->HasTalentTree(ability->m_talentTreeId), auraDebugInfo, "Passive aura '%s' affects '%s', which a '%s' can't have.",
											aura->m_name.c_str(), ability->m_name.c_str(), hybridTalentTree.m_string.c_str());

										bool required = false;
										for(uint32_t requiredTalentTreeId : passiveAura.m_requiredTalentTreeIds)
										{
											if(requiredTalentTreeId == ability->m_talentTreeId)
												required = true;
										}

										TP_VERIFY(required, auraDebugInfo, "Passive aura '%s' affects '%s', so it must have 'required_talent_trees' with '%s' (it might have been replaced).",
											aura->m_name.c_str(), ability->m_name.c_str(), aManifest->GetById<Data::TalentTree>(ability->m_talentTreeId)->m_name.c_str());
									}
								});
							}
						}
					}
				}
				return true;
			});
		}

	}

}
