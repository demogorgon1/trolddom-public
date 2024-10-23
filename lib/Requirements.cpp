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
#include <tpublic/Components/VisibleAuras.h>
#include <tpublic/Components/ZoneDiscovery.h>

#include <tpublic/Data/Aura.h>
#include <tpublic/Data/Entity.h>

#include <tpublic/EntityInstance.h>
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
			case Requirement::TYPE_MUST_HAVE_LESS_HEALTH_THAN:
				{
					const Components::CombatPublic* combatPublic = entity->GetComponent<Components::CombatPublic>();
					const Components::CombatPublic::ResourceEntry* health = combatPublic->GetResourceEntry(Resource::ID_HEALTH);
					if(health != NULL && health->m_max > 0)
					{
						uint32_t percent = (health->m_current * 100) / health->m_max;
						if(percent >= aRequirement->m_id)
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
					bool hasCompletedQuest = completedQuests->m_questIds.HasValue(aRequirement->m_id);
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

					if(!hasItem)
						return false;
				}
				break;

			case Requirement::TYPE_MUST_HAVE_DISCOVERED_ZONE:
				{
					if(!entity->IsPlayer())
						return false;

					const Components::ZoneDiscovery* zoneDiscovery = entity->GetComponent<Components::ZoneDiscovery>();
					if(!zoneDiscovery->m_zones.HasValue(aRequirement->m_id))
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
				{
					if (!entity->IsPlayer())
						return false;

					const Components::Reputation* reputation = entity->GetComponent<Components::Reputation>();
					int32_t factionReputation = reputation->GetReputation(aRequirement->m_id);
					if(factionReputation >= 0)
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

	}

}