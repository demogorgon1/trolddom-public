#include "../Pcheader.h"

#include <kaos-public/Components/Combat.h>
#include <kaos-public/Components/NPC.h>
#include <kaos-public/Components/Position.h>
#include <kaos-public/Components/Sprite.h>

#include <kaos-public/Systems/NPC.h>

#include <kaos-public/EntityInstance.h>
#include <kaos-public/Helpers.h>
#include <kaos-public/IMoveRequestQueue.h>
#include <kaos-public/IWorldView.h>

namespace kaos_public::Systems
{

	NPC::NPC(
		const Manifest*			aManifest)
		: SystemBase(aManifest)
	{
		RequireComponent<Components::Combat>();
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
		EntityState::Id			aEntityState,
		ComponentBase**			aComponents) 
	{
		Components::Combat* combat = GetComponent<Components::Combat>(aComponents);
		Components::NPC* npc = GetComponent<Components::NPC>(aComponents);
		Components::Sprite* sprite = GetComponent<Components::Sprite>(aComponents);

		const Components::NPC::StateEntry* state = npc->GetState(aEntityState);
		if(state != NULL)
		{
			sprite->m_spriteId = state->m_spriteId;
		}

		combat->m_currentHealth = npc->m_stats.m_stats[Stat::ID_HEALTH];
		combat->m_maxHealth = npc->m_stats.m_stats[Stat::ID_HEALTH];
	}

	EntityState::Id
	NPC::Update(
		EntityState::Id			aEntityState,
		ComponentBase**			aComponents,
		Context*				aContext) 
	{
		Components::Combat* combat = GetComponent<Components::Combat>(aComponents);
		Components::NPC* npc = GetComponent<Components::NPC>(aComponents);
		Components::Position* position = GetComponent<Components::Position>(aComponents);
		Components::Sprite* sprite = GetComponent<Components::Sprite>(aComponents);

		const Components::NPC::StateEntry* state = npc->GetState(aEntityState);
		if (state != NULL)
		{
			sprite->m_spriteId = state->m_spriteId;
		}

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
				else
				{
					const Components::Position* targetPosition = target->GetComponent<Components::Position>();

					int32_t dx = targetPosition->m_position.m_x - position->m_position.m_x;
					int32_t dy = targetPosition->m_position.m_y - position->m_position.m_y;

					if(abs(dx) + abs(dy) <= 1)
					{
						// FIXME: attack
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

