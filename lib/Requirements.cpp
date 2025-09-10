#include "Pcheader.h"

#include <tpublic/Components/ActiveQuests.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/CompletedQuests.h>
#include <tpublic/Components/EquippedItems.h>
#include <tpublic/Components/Inventory.h>
#include <tpublic/Components/Openable.h>
#include <tpublic/Components/PlayerPrivate.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/Reputation.h>
#include <tpublic/Components/SurvivalInfo.h>
#include <tpublic/Components/VisibleAuras.h>
#include <tpublic/Components/ZoneDiscovery.h>

#include <tpublic/Data/Aura.h>
#include <tpublic/Data/Entity.h>
#include <tpublic/Data/Objective.h>
#include <tpublic/Data/Quest.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/ItemProspect.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	namespace Requirements
	{

		bool	
		Check(
			const Manifest*						aManifest,
			const EntityInstance*				aSelf,
			const EntityInstance*				aTarget,
			const Requirement*					aRequirement)
		{
			const EntityInstance* entity = NULL;
			switch (aRequirement->m_target)
			{
			case Requirement::TARGET_SELF:	entity = aSelf; break;
			case Requirement::TARGET_TARGET: entity = aTarget; break;
			default: assert(false); break;
			}

			if (entity == NULL)
				return false;

			switch(aRequirement->m_type)
			{
			case Requirement::TYPE_MUST_BE_DISCIPLE:
			case Requirement::TYPE_MUST_NOT_BE_DISCIPLE:
				{
					if (!entity->IsPlayer())
						return false;

					const Components::Reputation* reputation = entity->GetComponent<Components::Reputation>();
					if (reputation == NULL)
						return false;

					bool shouldBeDisciple = aRequirement->m_type == Requirement::TYPE_MUST_BE_DISCIPLE;
					bool isDisciple = reputation->GetReputation(aRequirement->m_id) >= aManifest->m_worshipMetrics.m_discipleLevel;

					if(shouldBeDisciple != isDisciple)
						return false;
				}
				break;
			case Requirement::TYPE_MUST_BE_AT_LEAST_LEVEL:
				{
					const Components::CombatPublic* combatPublic = entity->GetComponent<Components::CombatPublic>();
					if(combatPublic == NULL || combatPublic->m_level < aRequirement->m_id)
						return false;
				}
				break;
			case Requirement::TYPE_MUST_HAVE_ITEM_TYPE_EQUIPPED:
				{
					if (!entity->IsPlayer())
						return false;

					const Components::EquippedItems* equippedItems = entity->GetComponent<Components::EquippedItems>();
					bool isEquipped = false;

					for(uint32_t i = 1; i < (uint32_t)EquipmentSlot::NUM_IDS; i++)
					{
						const ItemInstance& item = equippedItems->m_slots.m_items[i];
						if(item.IsSet())
						{
							const Data::Item* itemData = aManifest->GetById<Data::Item>(item.m_itemId);
							if(itemData->m_itemType == aRequirement->m_id)
							{
								isEquipped = true;
								break;
							}
						}
					}

					if(!isEquipped)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_HAVE_EQUIPPED_ITEM_TYPE_FLAGS:
			case Requirement::TYPE_MUST_NOT_HAVE_EQUIPPED_ITEM_TYPE_FLAGS:
			{
					if (!entity->IsPlayer())
						return false;

					const Components::PlayerPrivate* playerPrivate = entity->GetComponent<Components::PlayerPrivate>();
					bool isEquipped = (playerPrivate->m_equippedItemTypeFlags & (uint16_t)aRequirement->m_id) == (uint16_t)aRequirement->m_id;
					bool shouldBeEquipped = aRequirement->m_type == Requirement::TYPE_MUST_HAVE_EQUIPPED_ITEM_TYPE_FLAGS;
					if(isEquipped != shouldBeEquipped)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_BE_CREATURE_TYPE:
			case Requirement::TYPE_MUST_NOT_BE_CREATURE_TYPE:
				{
					const Components::CombatPublic* combatPublic = entity->GetComponent<Components::CombatPublic>();
					bool isCreatureType = combatPublic->m_creatureTypeId == aRequirement->m_id;
					bool shouldBeCreatureType = aRequirement->m_type == Requirement::TYPE_MUST_BE_CREATURE_TYPE;
					if(isCreatureType != shouldBeCreatureType)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_HAVE_LESS_HEALTH_THAN:
			case Requirement::TYPE_MUST_HAVE_ZERO_HEALTH:
				{
					const Components::CombatPublic* combatPublic = entity->GetComponent<Components::CombatPublic>();
					if(combatPublic != NULL)
					{
						const Components::CombatPublic::ResourceEntry* health = combatPublic->GetResourceEntry(Resource::ID_HEALTH);
						if (health != NULL && health->m_max > 0)
						{
							if (aRequirement->m_type == Requirement::TYPE_MUST_HAVE_LESS_HEALTH_THAN)
							{
								uint32_t percent = (health->m_current * 100) / health->m_max;
								if (percent >= aRequirement->m_id)
									return false;
							}
							else if (aRequirement->m_type == Requirement::TYPE_MUST_HAVE_ZERO_HEALTH)
							{
								if (health->m_current != 0)
									return false;
							}
						}
					}
				}
				break;

			case Requirement::TYPE_MUST_HAVE_MORE_RAGE_THAN:
				{
					const Components::CombatPublic* combatPublic = entity->GetComponent<Components::CombatPublic>();
					const Components::CombatPublic::ResourceEntry* rage = combatPublic->GetResourceEntry(Resource::ID_RAGE);
					if(rage != NULL && rage->m_max > 0)
					{
						if (rage->m_current < aRequirement->m_id)
							return false;
					}
				}
				break;

			case Requirement::TYPE_MUST_HAVE_AURA:
			case Requirement::TYPE_MUST_NOT_HAVE_AURA:
				{
					const Components::VisibleAuras* visibleAuras = entity->GetComponent<Components::VisibleAuras>();
					bool hasAura = visibleAuras->HasAura(aRequirement->m_id);
					if (aRequirement->m_type == Requirement::TYPE_MUST_HAVE_AURA && !hasAura)
						return false;
					else if (aRequirement->m_type == Requirement::TYPE_MUST_NOT_HAVE_AURA && hasAura)
						return false;
				}
				break;					

			case Requirement::TYPE_MUST_HAVE_AURA_GROUP:
				{
					const Components::VisibleAuras* visibleAuras = entity->GetComponent<Components::VisibleAuras>();
					bool hasAuraGroup = false;
					for(const Components::VisibleAuras::Entry& entry : visibleAuras->m_entries)
					{
						const Data::Aura* auraData = aManifest->GetById<Data::Aura>(entry.m_auraId);
						if(auraData->m_auraGroupId == aRequirement->m_id)
						{
							hasAuraGroup = true;
							break;
						}
					}

					if(!hasAuraGroup)
						return false;
				}
				break;					

			case Requirement::TYPE_MUST_HAVE_COMPLETED_QUEST:
			case Requirement::TYPE_MUST_NOT_HAVE_COMPLETED_QUEST:
				{
					if (!entity->IsPlayer())
						return false;

					const Components::CompletedQuests* completedQuests = entity->GetComponent<Components::CompletedQuests>();
					bool hasCompletedQuest = completedQuests->HasQuest(aRequirement->m_id);
					if(aRequirement->m_type == Requirement::TYPE_MUST_HAVE_COMPLETED_QUEST && !hasCompletedQuest)
						return false;
					else if (aRequirement->m_type == Requirement::TYPE_MUST_NOT_HAVE_COMPLETED_QUEST && hasCompletedQuest)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_HAVE_ACTIVE_QUEST:
			case Requirement::TYPE_MUST_NOT_HAVE_ACTIVE_QUEST:
				{
					if(!entity->IsPlayer())
						return false;

					const Components::ActiveQuests* activeQuests = entity->GetComponent<Components::ActiveQuests>();
					bool hasActiveQuest = activeQuests->HasQuest(aRequirement->m_id);
					if(aRequirement->m_type == Requirement::TYPE_MUST_HAVE_ACTIVE_QUEST && !hasActiveQuest)
						return false;
					else if (aRequirement->m_type == Requirement::TYPE_MUST_NOT_HAVE_ACTIVE_QUEST && hasActiveQuest)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_NOT_BE_READY_TO_TURN_IN_QUEST:
				{
					if (!entity->IsPlayer())
						return false;

					const Components::ActiveQuests* activeQuests = entity->GetComponent<Components::ActiveQuests>();
					const Components::ActiveQuests::Quest* quest = activeQuests->GetQuest(aRequirement->m_id);
					if(quest != NULL)
					{
						if(quest->m_objectiveInstanceData.empty())	
							return false;

						const Data::Quest* questData = aManifest->GetById<Data::Quest>(quest->m_questId);
						MemoryReader reader(&quest->m_objectiveInstanceData[0], quest->m_objectiveInstanceData.size());

						bool readyToTurnIn = true;

						for(uint32_t objectiveId : questData->m_objectives)
						{
							const Data::Objective* objectiveData = aManifest->GetById<Data::Objective>(objectiveId);
							std::unique_ptr<ObjectiveInstanceBase> objectiveInstanceBase(objectiveData->m_objectiveTypeBase->CreateInstance());
							if(!objectiveInstanceBase->FromStream(&reader))
								return false;

							if(!objectiveInstanceBase->IsCompleted())
							{
								readyToTurnIn = false;
								break;
							}
						}

						if(readyToTurnIn)
							return false;
					}
				}

				break;

			case Requirement::TYPE_MUST_HAVE_TAG:
				{
					if(entity->IsPlayer())
						return false;

					const Data::Entity* entityData = aManifest->GetById<Data::Entity>(entity->GetEntityId());
					if(!entityData->HasTag(aRequirement->m_id))
						return false;
				}
				break;

			case Requirement::TYPE_MUST_BE_TYPE:
				{
					if(entity->GetEntityId() != aRequirement->m_id)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_NOT_BE_TYPE:
				{
					if(entity->GetEntityId() == aRequirement->m_id)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_BE_IN_STATE:
				{
					if(entity->GetState() != (EntityState::Id)aRequirement->m_id)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_NOT_BE_IN_STATE:
				{
					if(entity->GetState() == (EntityState::Id)aRequirement->m_id)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_BE_HOSTILE:
				{
					const Components::CombatPublic* combatPublic = entity->GetComponent<Components::CombatPublic>();
					const Components::CombatPublic* selfCombatPublic = aSelf->GetComponent<Components::CombatPublic>();
					if(combatPublic->m_factionId == selfCombatPublic->m_factionId)				
						return false;	
				}
				break;

			case Requirement::TYPE_MUST_BE_FACTION:
				{
					const Components::CombatPublic* combatPublic = entity->GetComponent<Components::CombatPublic>();
					if(combatPublic->m_factionId != aRequirement->m_id)
						return false;	
				}
				break;

			case Requirement::TYPE_MUST_BE_GROUP_MEMBER:
				{
					if(entity->GetEntityInstanceId() != aSelf->GetEntityInstanceId())
					{
						const Components::CombatPublic* combatPublic = entity->GetComponent<Components::CombatPublic>();
						if(combatPublic->m_combatGroupId == 0)
							return false;

						const Components::CombatPublic* selfCombatPublic = aSelf->GetComponent<Components::CombatPublic>();
						if (combatPublic->m_combatGroupId != selfCombatPublic->m_combatGroupId)
							return false;
					}
				}
				break;

			case Requirement::TYPE_MUST_HAVE_ITEM_EQUIPPED:
			case Requirement::TYPE_MUST_NOT_HAVE_ITEM_EQUIPPED:
				{
					if (!entity->IsPlayer())
						return false;

					const Components::EquippedItems* equippedItems = entity->GetComponent<Components::EquippedItems>();
					const Data::Item* item = aManifest->GetById<Data::Item>(aRequirement->m_id);
					bool isEquipped = false;

					for(uint32_t equipmentSlotId : item->m_equipmentSlots)
					{
						if(equippedItems->m_slots.m_items[equipmentSlotId].m_itemId == aRequirement->m_id)
						{
							isEquipped = true;
							break;
						}
					}

					bool shouldBeEquipped = aRequirement->m_type == Requirement::TYPE_MUST_HAVE_ITEM_EQUIPPED;

					if(isEquipped != shouldBeEquipped)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_HAVE_ITEM:
			case Requirement::TYPE_MUST_NOT_HAVE_ITEM:
				{
					if (!entity->IsPlayer())
						return false;

					const Components::EquippedItems* equippedItems = entity->GetComponent<Components::EquippedItems>();
					const Data::Item* item = aManifest->GetById<Data::Item>(aRequirement->m_id);
					bool hasItem = false;

					for(uint32_t equipmentSlotId : item->m_equipmentSlots)
					{
						if(equippedItems->m_slots.m_items[equipmentSlotId].m_itemId == aRequirement->m_id)
						{
							hasItem = true;
							break;
						}
					}

					if(!hasItem)
					{
						const Components::Inventory* inventory = entity->GetComponent<Components::Inventory>();
						hasItem = inventory->m_itemList.HasItems(aRequirement->m_id, 1);
					}

					bool shouldHaveItem = aRequirement->m_type == Requirement::TYPE_MUST_HAVE_ITEM;

					if(hasItem != shouldHaveItem)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_HAVE_DISCOVERED_ZONE:
			case Requirement::TYPE_MUST_NOT_HAVE_DISCOVERED_ZONE:
			{
					if(!entity->IsPlayer())
						return false;

					const Components::ZoneDiscovery* zoneDiscovery = entity->GetComponent<Components::ZoneDiscovery>();
					bool hasDiscoveredZone = zoneDiscovery->m_zones.HasValue(aRequirement->m_id);
					bool shouldHaveDiscoveredZone = aRequirement->m_type == Requirement::TYPE_MUST_HAVE_DISCOVERED_ZONE;

					if(hasDiscoveredZone != shouldHaveDiscoveredZone)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_NOT_HAVE_PROFESSION_ABILITY:
				{
					if(!entity->IsPlayer())
						return false;

					const Data::Ability* ability = aManifest->GetById<Data::Ability>(aRequirement->m_id);
					if(!ability->m_requiredProfession.has_value())
						return false;

					const Components::PlayerPrivate* playerPrivate = entity->GetComponent<Components::PlayerPrivate>();
					const PlayerProfessions::Entry* entry = playerPrivate->m_professions.GetProfession(ability->m_requiredProfession->m_professionId);
					if(entry == NULL)
						return false;

					if(entry->HasAbility(ability->m_id))
						return false;
				}				
				break;

			case Requirement::TYPE_MUST_HAVE_NEGATIVE_REPUTATION:
			case Requirement::TYPE_MUST_HAVE_REPUTATION_LEVEL:
				{
					if (!entity->IsPlayer())
						return false;

					const Components::Reputation* reputation = entity->GetComponent<Components::Reputation>();
					int32_t factionReputation = reputation->GetReputation(aRequirement->m_id);

					if(aRequirement->m_type == Requirement::TYPE_MUST_HAVE_REPUTATION_LEVEL)
					{
						uint32_t levelIndex = aManifest->m_reputationMetrics.GetLevelIndexFromReputation(factionReputation);
						if(levelIndex < aRequirement->m_value)
							return false;							
					}
					else
					{
						if (factionReputation >= 0)
							return false;
					}
				}
				break;

			case Requirement::TYPE_MUST_KNOW_RIDING:
			case Requirement::TYPE_MUST_NOT_KNOW_RIDING:
				{
					if (!entity->IsPlayer())
						return false;

					const Components::PlayerPrivate* playerPrivate = entity->GetComponent<Components::PlayerPrivate>();
					bool mustKnowRiding = aRequirement->m_type == Requirement::TYPE_MUST_KNOW_RIDING;

					if(playerPrivate->m_knowsRiding != mustKnowRiding)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_HAVE_STARTED_SURVIVAL:
			case Requirement::TYPE_MUST_NOT_HAVE_STARTED_SURVIVAL:
				{
					if (!entity->IsPlayer())
						return false;

					const Components::SurvivalInfo* survivalInfo = entity->GetComponent<Components::SurvivalInfo>();
					bool shouldBeStarted = aRequirement->m_type == Requirement::TYPE_MUST_HAVE_STARTED_SURVIVAL;
					bool isStarted = survivalInfo->m_state == Survival::STATE_STARTED;

					if (shouldBeStarted != isStarted)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_BE_ON_MAP:
			case Requirement::TYPE_MUST_NOT_BE_ON_MAP:
				{
					if (!entity->IsPlayer())
						return false;

					const Components::PlayerPrivate* playerPrivate = entity->GetComponent<Components::PlayerPrivate>();

					bool shouldBeOnMap = aRequirement->m_type == Requirement::TYPE_MUST_BE_ON_MAP;
					bool isOnMap = playerPrivate->m_mapId == aRequirement->m_id;

					if(shouldBeOnMap != isOnMap)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_HAVE_FISHING_ROD_EQUIPPED:
				{
					if (!entity->IsPlayer())
						return false;

					const Components::EquippedItems* equippedItems = entity->GetComponent<Components::EquippedItems>();
					bool isEquipped = false;

					const ItemInstance& item = equippedItems->m_slots.m_items[EquipmentSlot::ID_MAIN_HAND];
					if(item.IsSet())
					{
						const Data::Item* itemData = aManifest->GetById<Data::Item>(item.m_itemId);
						if(itemData->m_flags & Data::Item::FLAG_FISHING_ROD)
							isEquipped = true;
					}

					if(!isEquipped)
						return false;
				}
				break;

			default:
				assert(false);
				break;
			}

			return true;
		}

		bool	
		CheckList(
			const Manifest*										aManifest,
			const std::vector<Requirement>&						aRequirements,
			const EntityInstance*								aSelf,
			const EntityInstance*								aTarget,
			const Requirement**									aOutFailedRequirement)
		{
			for(const Requirement& requirement : aRequirements)
			{
				if(!Check(aManifest, aSelf, aTarget, &requirement))
				{
					if(aOutFailedRequirement != NULL)
						*aOutFailedRequirement = &requirement;
					return false;
				}
			}

			return true;
		}

		bool	
		CheckListUnresolved(
			const Manifest*										aManifest,
			const std::vector<Requirement>&						aRequirements,
			const IWorldView*									aWorldView,
			uint32_t											aSelfEntityInstanceId,
			uint32_t											aTargetEntityInstanceId)
		{
			const EntityInstance* self = aWorldView->WorldViewSingleEntityInstance(aSelfEntityInstanceId);
			const EntityInstance* target = aWorldView->WorldViewSingleEntityInstance(aTargetEntityInstanceId);

			return CheckList(aManifest, aRequirements, self, target, NULL);
		}

		bool	
		CheckAnyList(
			const Manifest*										aManifest,
			const std::vector<Requirement>&						aRequirements,
			const std::vector<const EntityInstance*>&			aSelves,
			const EntityInstance*								aTarget)
		{
			for(const EntityInstance* self : aSelves)
			{
				bool ok = true;

				for (const Requirement& requirement : aRequirements)
				{
					if (!Check(aManifest, self, aTarget, &requirement))
					{
						ok = false;
						break;
					}
				}

				if(ok)
					return true;
			}
			return false;
		}

		bool	
		CheckOpenable(
			const Manifest*										aManifest,
			const EntityInstance*								aSelf,
			const EntityInstance*								aTarget,
			bool*												aOutInstant,
			bool*												aOutOutOfRange)
		{
			const Components::Openable* openable = aTarget->GetComponent<Components::Openable>();
			if(openable == NULL)
				return false;

			if (openable->m_requiredProfessionId != 0)
			{
				const tpublic::Components::PlayerPrivate* playerPrivate = aSelf->GetComponent<tpublic::Components::PlayerPrivate>();
				if(!playerPrivate->m_professions.HasProfessionSkill(openable->m_requiredProfessionId, openable->m_requiredProfessionSkill))
					return false;
			}

			if (openable->m_requiredItemId != 0)
			{
				const tpublic::Components::Inventory* inventory = aSelf->GetComponent<tpublic::Components::Inventory>();
				if(!inventory->m_itemList.HasItems(openable->m_requiredItemId, 1))
				{
					const tpublic::Components::EquippedItems* equippedItems = aSelf->GetComponent<tpublic::Components::EquippedItems>();
					bool hasItem = false;
					equippedItems->ForEach([&](
						const tpublic::ItemInstance& aItemInstance)
					{
						if(aItemInstance.m_itemId == openable->m_requiredItemId)
							hasItem = true;
					});
					
					if(!hasItem)
						return false;
				}
			}

			if(openable->m_requiredQuestId)
			{
				const tpublic::Components::ActiveQuests* activeQuests = aSelf->GetComponent<tpublic::Components::ActiveQuests>();
				if(!activeQuests->HasQuest(openable->m_requiredQuestId))
					return false;
			}

			if(!CheckList(aManifest, openable->m_requirements, aSelf, aTarget, NULL))
				return false;

			const Components::Position* selfPosition = aSelf->GetComponent<Components::Position>();
			const Components::Position* targetPosition = aTarget->GetComponent<Components::Position>();
			bool outOfRange = false;

			if(openable->m_range == 1)
			{
				// Special case for range 1: can also do neighboring diagonals
				Vec2 d = selfPosition->m_position - targetPosition->m_position;
				outOfRange = d.m_x < -1 || d.m_y < -1 || d.m_x > 1 || d.m_y > 1;
			}
			else
			{
				int32_t distanceSquared = Helpers::CalculateDistanceSquared(selfPosition, targetPosition);
				outOfRange = distanceSquared > (int32_t)(openable->m_range * openable->m_range);
			}

			if(outOfRange)
			{
				if (aOutOutOfRange != NULL)
					*aOutOutOfRange = true;

				return false;
			}

			if(aOutInstant != NULL)
				*aOutInstant = openable->m_instant;

			return true;
		}

		bool	
		CheckTargetItemRequirements(
			const Manifest*									aManifest,
			const Data::Ability::TargetItemRequirements*	aTargetItemRequirements,
			const ItemProspect*								aTargetItemProspect,
			uint32_t										aItemId)
		{
			if(aTargetItemRequirements == NULL)
				return true;

			const Data::Item* item = aManifest->GetById<Data::Item>(aItemId);

			if(aTargetItemRequirements->m_mustBeSellable && item->IsNotSellable())
				return false;

			if(aTargetItemProspect != NULL && !aTargetItemProspect->CanResolve(item))
				return false;
			
			if(!aTargetItemRequirements->m_equipmentSlots.empty())
			{
				bool ok = false;

				for(EquipmentSlot::Id equipmentSlot : aTargetItemRequirements->m_equipmentSlots)
				{
					if(item->IsEquippableInSlot(equipmentSlot))
					{
						ok = true;
						break;
					}
				}

				if(!ok)
					return false;
			}

			if (!aTargetItemRequirements->m_rarities.empty())
			{
				bool ok = false;

				for (Rarity::Id rarity : aTargetItemRequirements->m_rarities)
				{
					if(item->m_rarity == rarity)
					{
						ok = true;
						break;
					}
				}

				if (!ok)
					return false;
			}

			if (!aTargetItemRequirements->m_itemTypes.empty())
			{
				bool ok = false;

				for (ItemType::Id itemType : aTargetItemRequirements->m_itemTypes)
				{
					if (item->m_itemType == itemType)
					{
						ok = true;
						break;
					}
				}

				if (!ok)
					return false;
			}

			return true;
		}

	}

}