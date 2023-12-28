#include "Pcheader.h"

#include <tpublic/Components/ActiveQuests.h>
#include <tpublic/Components/CompletedQuests.h>
#include <tpublic/Components/EquippedItems.h>
#include <tpublic/Components/Inventory.h>
#include <tpublic/Components/Openable.h>
#include <tpublic/Components/PlayerPrivate.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/VisibleAuras.h>

#include <tpublic/EntityInstance.h>

namespace tpublic
{

	namespace Requirements
	{

		bool	
		Check(
			const EntityInstance*				aSelf,
			const EntityInstance*				aTarget,
			const Data::Ability::Requirement*	aRequirement)
		{
			const EntityInstance* entity = NULL;
			switch (aRequirement->m_target)
			{
			case Data::Ability::Requirement::TARGET_SELF:	entity = aSelf; break;
			case Data::Ability::Requirement::TARGET_TARGET: entity = aTarget; break;
			default: assert(false); break;
			}

			if (entity == NULL)
				return false;

			switch(aRequirement->m_type)
			{
			case Data::Ability::Requirement::TYPE_MUST_HAVE_AURA:
			case Data::Ability::Requirement::TYPE_MUST_NOT_HAVE_AURA:
				{
					const Components::VisibleAuras* visibleAuras = entity->GetComponent<Components::VisibleAuras>();
					bool hasAura = visibleAuras->HasAura(aRequirement->m_id);
					if (aRequirement->m_type == Data::Ability::Requirement::TYPE_MUST_HAVE_AURA && !hasAura)
						return false;
					else if (aRequirement->m_type == Data::Ability::Requirement::TYPE_MUST_NOT_HAVE_AURA && hasAura)
						return false;
				}
				break;					

			case Data::Ability::Requirement::TYPE_MUST_HAVE_COMPLETED_QUEST:
			case Data::Ability::Requirement::TYPE_MUST_NOT_HAVE_COMPLETED_QUEST:
				{
					if(aRequirement->m_target != Data::Ability::Requirement::TARGET_SELF)
						return false;

					const Components::CompletedQuests* completedQuests = entity->GetComponent<Components::CompletedQuests>();
					bool hasCompletedQuest = completedQuests->m_questIds.HasValue(aRequirement->m_id);
					if(aRequirement->m_type == Data::Ability::Requirement::TYPE_MUST_HAVE_COMPLETED_QUEST && !hasCompletedQuest)
						return false;
					else if (aRequirement->m_type == Data::Ability::Requirement::TYPE_MUST_NOT_HAVE_COMPLETED_QUEST && hasCompletedQuest)
						return false;
				}
				break;

			case Data::Ability::Requirement::TYPE_MUST_HAVE_ACTIVE_QUEST:
			case Data::Ability::Requirement::TYPE_MUST_NOT_HAVE_ACTIVE_QUEST:
				{
					if(aRequirement->m_target != Data::Ability::Requirement::TARGET_SELF)
						return false;

					const Components::ActiveQuests* activeQuests = entity->GetComponent<Components::ActiveQuests>();
					bool hasActiveQuest = activeQuests->HasQuest(aRequirement->m_id);
					if(aRequirement->m_type == Data::Ability::Requirement::TYPE_MUST_HAVE_ACTIVE_QUEST && !hasActiveQuest)
						return false;
					else if (aRequirement->m_type == Data::Ability::Requirement::TYPE_MUST_NOT_HAVE_ACTIVE_QUEST && hasActiveQuest)
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
		CheckAbility(
			const Data::Ability*				aAbility,
			const EntityInstance*				aSelf,
			const EntityInstance*				aTarget,
			const Data::Ability::Requirement**	aOutFailedRequirement)
		{
			for(const Data::Ability::Requirement& requirement : aAbility->m_requirements)
			{
				if(!Check(aSelf, aTarget, &requirement))
				{
					if(aOutFailedRequirement != NULL)
						*aOutFailedRequirement = &requirement;
					return false;
				}
			}

			return true;
		}

		bool	
		CheckOpenable(
			const EntityInstance*			aSelf,
			const EntityInstance*			aTarget,
			bool*							aOutInstant)
		{
			const Components::Openable* openable = aTarget->GetComponent<Components::Openable>();
			if(openable == NULL)
				return false;

			const Components::Position* selfPosition = aSelf->GetComponent<Components::Position>();
			const Components::Position* targetPosition = aTarget->GetComponent<Components::Position>();
			if(selfPosition->m_position.DistanceSquared(targetPosition->m_position) > (int32_t)(openable->m_range * openable->m_range))
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

			if(aOutInstant != NULL)
				*aOutInstant = openable->m_instant;

			return true;
		}

	}

}