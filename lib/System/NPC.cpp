#include "../Pcheader.h"

#include <kaos-public/Components/CombatPublic.h>
#include <kaos-public/Components/NPC.h>
#include <kaos-public/Components/Position.h>
#include <kaos-public/Components/Sprite.h>

#include <kaos-public/Systems/NPC.h>

#include <kaos-public/EntityInstance.h>
#include <kaos-public/Helpers.h>
#include <kaos-public/ICombatEventQueue.h>
#include <kaos-public/IMoveRequestQueue.h>
#include <kaos-public/IWorldView.h>
#include <kaos-public/Manifest.h>

namespace kaos_public::Systems
{

	NPC::NPC(
		const Manifest*			aManifest)
		: SystemBase(aManifest)
	{
		RequireComponent<Components::CombatPublic>();
		RequireComponent<Components::NPC>();
		RequireComponent<Components::Position>();
		RequireComponent<Components::Sprite>();
	}
	
	NPC::~NPC()
	{

	}

	//--------------------------------------------------------------------------------------------

	void		
	NPC::Init(
		uint32_t				/*aEntityInstanceId*/,
		EntityState::Id			aEntityState,
		ComponentBase**			aComponents) 
	{
		Components::CombatPublic* combat = GetComponent<Components::CombatPublic>(aComponents);
		Components::NPC* npc = GetComponent<Components::NPC>(aComponents);
		Components::Sprite* sprite = GetComponent<Components::Sprite>(aComponents);

		const Components::NPC::StateEntry* state = npc->GetState(aEntityState);
		if(state != NULL)
		{
			sprite->m_spriteId = state->m_spriteId;
		}

		for(const Components::NPC::ResourceEntry& resource : npc->m_resources)
			combat->AddResourceMax(resource.m_id, resource.m_max);
	}

	EntityState::Id
	NPC::Update(
		uint32_t				aEntityInstanceId,
		EntityState::Id			aEntityState,
		ComponentBase**			aComponents,
		Context*				aContext) 
	{
		Components::CombatPublic* combat = GetComponent<Components::CombatPublic>(aComponents);
		Components::NPC* npc = GetComponent<Components::NPC>(aComponents);
		Components::Position* position = GetComponent<Components::Position>(aComponents);
		Components::Sprite* sprite = GetComponent<Components::Sprite>(aComponents);

		const Components::NPC::StateEntry* state = npc->GetState(aEntityState);
		if (state != NULL)
		{
			sprite->m_spriteId = state->m_spriteId;
		}

		npc->m_cooldowns.Update(aContext->m_tick);

		EntityState::Id returnValue = EntityState::CONTINUE;

		switch(aEntityState)
		{
		case EntityState::ID_DEFAULT:
			aContext->m_worldView->QueryAllEntityInstances([&](
				const EntityInstance* aEntity)
			{
				if(aEntity->IsPlayer())
				{
					if(Helpers::IsWithinDistance(aEntity->GetComponent<Components::Position>(), position, 3))
					{
						combat->m_targetEntityInstanceId = aEntity->GetEntityInstanceId();

						returnValue = EntityState::ID_IN_COMBAT;
					}
				}

				return false;
			});
			break;

		case EntityState::ID_IN_COMBAT:
			{
				const EntityInstance* target = aContext->m_worldView->QuerySingleEntityInstance(combat->m_targetEntityInstanceId);
				if(target == NULL)
				{
					combat->m_targetEntityInstanceId = 0;

					returnValue = EntityState::ID_DEFAULT;
				}
				else if(state != NULL)
				{
					const Components::Position* targetPosition = target->GetComponent<Components::Position>();

					int32_t dx = targetPosition->m_position.m_x - position->m_position.m_x;
					int32_t dy = targetPosition->m_position.m_y - position->m_position.m_y;
					int32_t distanceSquared = dx * dx + dy * dy;

					const Data::Ability* useAbility = NULL;

					for(const Components::NPC::AbilityEntry& abilityEntry : state->m_abilities)
					{
						const Data::Ability* ability = GetManifest()->m_abilities.GetById(abilityEntry.m_abilityId);

						if(distanceSquared <= (int32_t)(ability->m_range * ability->m_range) && npc->m_cooldowns.Get(ability->m_id) == NULL)
						{
							if(abilityEntry.m_useProbability == UINT32_MAX || (*aContext->m_random)() < abilityEntry.m_useProbability)
								useAbility = ability;
						}
					}

					if(useAbility != NULL)
					{
						npc->m_cooldowns.Add(useAbility, aContext->m_tick);

						aContext->m_combatEventQueue->AddCombatEvent(aEntityInstanceId, target->GetEntityInstanceId(), useAbility);
					}
					else
					{
						aContext->m_moveRequestQueue->AddMoveRequest(position, Vec2(dx, dy));
					}
				}
			}
			break;

		default:
			break;
		}

		return returnValue;
	}

}

