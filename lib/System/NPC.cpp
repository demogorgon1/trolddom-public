#include "../Pcheader.h"

#include <kpublic/Components/CombatPublic.h>
#include <kpublic/Components/NPC.h>
#include <kpublic/Components/Position.h>
#include <kpublic/Components/Sprite.h>
#include <kpublic/Components/ThreatTarget.h>

#include <kpublic/Systems/NPC.h>

#include <kpublic/EntityInstance.h>
#include <kpublic/Helpers.h>
#include <kpublic/IAbilityQueue.h>
#include <kpublic/IMoveRequestQueue.h>
#include <kpublic/IWorldView.h>
#include <kpublic/Manifest.h>

namespace kpublic::Systems
{

	NPC::NPC(
		const Manifest*			aManifest)
		: SystemBase(aManifest)
	{
		RequireComponent<Components::CombatPublic>();
		RequireComponent<Components::NPC>();
		RequireComponent<Components::Position>();
		RequireComponent<Components::Sprite>();
		RequireComponent<Components::ThreatTarget>();
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
		Components::ThreatTarget* threat = GetComponent<Components::ThreatTarget>(aComponents);

		const Components::NPC::StateEntry* state = npc->GetState(aEntityState);
		if (state != NULL)
		{
			sprite->m_spriteId = state->m_spriteId;
		}			

		if (aEntityState != EntityState::ID_DEAD && combat->GetResource(Resource::ID_HEALTH) == 0)
		{
			return EntityState::ID_DEAD;
		}

		npc->m_cooldowns.Update(aContext->m_tick);
		threat->m_table.Update(aContext->m_tick);

		EntityState::Id returnValue = EntityState::CONTINUE;

		if(aEntityState != EntityState::ID_DEAD)
		{
			if(aContext->m_tick - threat->m_lastPingTick >= Components::ThreatTarget::PING_INTERVAL_TICKS)
			{
				aContext->m_worldView->QueryAllEntityInstances([&](
					const EntityInstance* aEntity)
				{
					if (aEntity->IsPlayer())
					{
						if (Helpers::IsWithinDistance(aEntity->GetComponent<Components::Position>(), position, 3))
							aContext->m_threatEventQueue->AddThreatEvent(aEntity->GetEntityInstanceId(), aEntityInstanceId, 0);
					}

					return false;
				});

				threat->m_lastPingTick = aContext->m_tick;
			}
		}
		
		switch(aEntityState)
		{
		case EntityState::ID_DEFAULT:
			if (!threat->m_table.IsEmpty())
			{
				combat->m_targetEntityInstanceId = threat->m_table.GetTop()->m_entityInstanceId;
				returnValue = EntityState::ID_IN_COMBAT;
			}
			break;

		case EntityState::ID_IN_COMBAT:
			{
				if (threat->m_table.IsEmpty())
				{
					combat->m_targetEntityInstanceId = 0;

					returnValue = EntityState::ID_DEFAULT;
				}
				else 
				{
					combat->m_targetEntityInstanceId = threat->m_table.GetTop()->m_entityInstanceId;

					const EntityInstance* target = aContext->m_worldView->QuerySingleEntityInstance(combat->m_targetEntityInstanceId);
					if (target == NULL)
					{
						combat->m_targetEntityInstanceId = 0;

						returnValue = EntityState::ID_DEFAULT;
					}
					else if (state != NULL)
					{
						const Components::Position* targetPosition = target->GetComponent<Components::Position>();

						int32_t dx = targetPosition->m_position.m_x - position->m_position.m_x;
						int32_t dy = targetPosition->m_position.m_y - position->m_position.m_y;
						int32_t distanceSquared = dx * dx + dy * dy;

						const Data::Ability* useAbility = NULL;

						for (const Components::NPC::AbilityEntry& abilityEntry : state->m_abilities)
						{
							const Data::Ability* ability = GetManifest()->m_abilities.GetById(abilityEntry.m_abilityId);

							if (distanceSquared <= (int32_t)(ability->m_range * ability->m_range) && npc->m_cooldowns.Get(ability->m_id) == NULL)
							{
								if (abilityEntry.m_useProbability == UINT32_MAX || (*aContext->m_random)() < abilityEntry.m_useProbability)
									useAbility = ability;
							}
						}

						if (useAbility != NULL)
						{
							npc->m_cooldowns.Add(useAbility, aContext->m_tick);

							aContext->m_abilityQueue->AddAbility(aEntityInstanceId, target->GetEntityInstanceId(), useAbility);
						}
						else
						{
							aContext->m_moveRequestQueue->AddMoveRequest(position, Vec2(dx, dy));
						}
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

