#include "../Pcheader.h"

#include <kaos-public/Components/Combat.h>
#include <kaos-public/Components/NPC.h>
#include <kaos-public/Components/Position.h>
#include <kaos-public/Components/Sprite.h>

#include <kaos-public/Systems/NPC.h>

#include <kaos-public/EntityInstance.h>
#include <kaos-public/Helpers.h>
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
						returnValue = EntityState::ID_IN_COMBAT;
					}
				}

				return false;
			});
			break;

		case EntityState::ID_IN_COMBAT:
			break;

		default:
			break;
		}

		(void)combat;

		return returnValue;
	}

}

