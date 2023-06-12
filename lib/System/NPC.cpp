#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/NPC.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/Sprite.h>
#include <tpublic/Components/ThreatTarget.h>

#include <tpublic/Systems/NPC.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Helpers.h>
#include <tpublic/IAbilityQueue.h>
#include <tpublic/IMoveRequestQueue.h>
#include <tpublic/IThreatEventQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Manifest.h>

namespace tpublic::Systems
{

	NPC::NPC(
		const Manifest*			aManifest)
		: SystemBase(aManifest)
	{
		RequireComponent<Components::Auras>();
		RequireComponent<Components::CombatPublic>();
		RequireComponent<Components::NPC>();
		RequireComponent<Components::Position>();
		RequireComponent<Components::ThreatTarget>();
	}
	
	NPC::~NPC()
	{

	}

	//--------------------------------------------------------------------------------------------

	void		
	NPC::Init(
		uint32_t				/*aEntityInstanceId*/,
		EntityState::Id			/*aEntityState*/,
		ComponentBase**			aComponents,
		int32_t					/*aTick*/)
	{
		Components::CombatPublic* combat = GetComponent<Components::CombatPublic>(aComponents);
		Components::NPC* npc = GetComponent<Components::NPC>(aComponents);

		for(const Components::NPC::ResourceEntry& resource : npc->m_resources)
		{
			combat->AddResourceMax(resource.m_id, resource.m_max);

			const Resource::Info* info = Resource::GetInfo((Resource::Id)resource.m_id);
			if(info->m_flags & Resource::FLAG_DEFAULT_TO_MAX)
				combat->SetResourceToMax(resource.m_id);
		}
	}

	EntityState::Id
	NPC::UpdatePrivate(
		uint32_t				aEntityInstanceId,
		EntityState::Id			aEntityState,
		int32_t					aTicksInState,
		ComponentBase**			aComponents,
		Context*				aContext) 
	{		
		if(aEntityState == EntityState::ID_SPAWNING)
			return aTicksInState < SPAWN_TICKS ? EntityState::CONTINUE : EntityState::ID_DEFAULT;

		if (aEntityState == EntityState::ID_DESPAWNING)
			return aTicksInState < DESPAWN_TICKS ? EntityState::CONTINUE : EntityState::DESTROY;

		const Components::CombatPublic* combat = GetComponent<Components::CombatPublic>(aComponents);
		Components::NPC* npc = GetComponent<Components::NPC>(aComponents);
		const Components::Position* position = GetComponent<Components::Position>(aComponents);
		Components::ThreatTarget* threat = GetComponent<Components::ThreatTarget>(aComponents);
		const Components::Auras* auras = GetComponent<Components::Auras>(aComponents);
		const Components::NPC::StateEntry* state = npc->GetState(aEntityState);

		if (aEntityState != EntityState::ID_DEAD && combat->GetResource(Resource::ID_HEALTH) == 0 && !auras->HasEffect(AuraEffect::ID_IMMORTALITY))
		{
			npc->m_castInProgress.reset();
			return EntityState::ID_DEAD;
		}

		if(aEntityState == EntityState::ID_DEAD && !threat->m_table.IsEmpty())
			aContext->m_threatEventQueue->AddThreatClearEvent(aEntityInstanceId);

		npc->m_cooldowns.Update(aContext->m_tick);

		std::vector<uint32_t> threatRemovedEntityInstanceIds;
		threat->m_table.Update(aContext->m_tick, threatRemovedEntityInstanceIds);

		for(uint32_t threatRemovedInstanceId : threatRemovedEntityInstanceIds)
			aContext->m_threatEventQueue->AddThreatEvent(threatRemovedInstanceId, aEntityInstanceId, INT32_MIN);

		EntityState::Id returnValue = EntityState::CONTINUE;

		if(aEntityState != EntityState::ID_DEAD)
		{
			if(npc->m_castInProgress && aContext->m_tick >= npc->m_castInProgress->m_end)
			{				
				aContext->m_abilityQueue->AddAbility(
					aEntityInstanceId, 
					npc->m_castInProgress->m_targetEntityInstanceId,
					npc->m_castInProgress->m_aoeTarget,
					GetManifest()->m_abilities.GetById(npc->m_castInProgress->m_abilityId));
				npc->m_castInProgress.reset();
			}

			const EntityInstance* topThreatEntity = NULL;
			if(!threat->m_table.IsEmpty())
			{
				topThreatEntity = aContext->m_worldView->QuerySingleEntityInstance(threat->m_table.GetTop()->m_entityInstanceId);
				if(topThreatEntity != NULL && topThreatEntity->GetState() == EntityState::ID_DEAD)
					topThreatEntity = NULL;
			}

			if(aContext->m_tick - threat->m_lastPingTick >= Components::ThreatTarget::PING_INTERVAL_TICKS)
			{
				aContext->m_worldView->QueryAllEntityInstances([&](
					const EntityInstance* aEntity)
				{
					if(aEntity->GetState() != EntityState::ID_DEAD)
					{
						if (aEntity->IsPlayer())
						{
							if (Helpers::IsWithinDistance(aEntity->GetComponent<Components::Position>(), position, 3))
								aContext->m_threatEventQueue->AddThreatEvent(aEntity->GetEntityInstanceId(), aEntityInstanceId, 0);
						}
						else if(topThreatEntity != NULL && aEntity->GetState() != EntityState::ID_IN_COMBAT)
						{
							if (Helpers::IsWithinDistance(aEntity->GetComponent<Components::Position>(), position, 3))
							{
								const tpublic::Components::CombatPublic* nearbyNonPlayerCombatPublic = aEntity->GetComponent<tpublic::Components::CombatPublic>();
								if(nearbyNonPlayerCombatPublic != NULL && nearbyNonPlayerCombatPublic->m_factionId == combat->m_factionId)
									aContext->m_threatEventQueue->AddThreatEvent(topThreatEntity->GetEntityInstanceId(), aEntity->GetEntityInstanceId(), 0);
							}
						}
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
				threat->m_lastPingTick = 0; // Force a threat ping (will cause nearby allies to join)

				npc->m_targetEntityInstanceId = threat->m_table.GetTop()->m_entityInstanceId;
				returnValue = EntityState::ID_IN_COMBAT;
			}
			break;

		case EntityState::ID_IN_COMBAT:
			{
				if (threat->m_table.IsEmpty())
				{
					npc->m_targetEntityInstanceId = 0;
					npc->m_castInProgress.reset();

					returnValue = EntityState::ID_DEFAULT;
				}
				else if(auras->HasEffect(AuraEffect::ID_STUN))
				{
					npc->m_castInProgress.reset();
				}
				else
				{
					npc->m_targetEntityInstanceId = threat->m_table.GetTop()->m_entityInstanceId;

					const EntityInstance* target = aContext->m_worldView->QuerySingleEntityInstance(npc->m_targetEntityInstanceId);
					if (target == NULL || target->GetState() == EntityState::ID_DEAD)
					{
						threat->m_table.Remove(npc->m_targetEntityInstanceId);
						npc->m_castInProgress.reset();
						npc->m_targetEntityInstanceId = 0;
					}
					else if (state != NULL && !npc->m_castInProgress)						
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

							if(useAbility->m_castTime > 0)
							{
								CastInProgress cast;
								cast.m_abilityId = useAbility->m_id;
								cast.m_targetEntityInstanceId = target->GetEntityInstanceId();
								cast.m_start = aContext->m_tick;
								cast.m_end = cast.m_start + useAbility->m_castTime;
								npc->m_castInProgress = cast;
							}
							else
							{
								aContext->m_abilityQueue->AddAbility(aEntityInstanceId, target->GetEntityInstanceId(), Vec2(), useAbility);
							}
						}
						else if(npc->m_moveCooldownUntilTick < aContext->m_tick)
						{
							aContext->m_moveRequestQueue->AddMoveRequest(aEntityInstanceId, Vec2(dx, dy));

							npc->m_moveCooldownUntilTick = aContext->m_tick + 2;
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

	void			
	NPC::UpdatePublic(
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			/*aContext*/) 
	{
		Components::CombatPublic* combat = GetComponent<Components::CombatPublic>(aComponents);
		const Components::NPC* npc = GetComponent<Components::NPC>(aComponents);
		Components::Position* position = GetComponent<Components::Position>(aComponents);

		combat->m_targetEntityInstanceId = npc->m_targetEntityInstanceId;

		if(npc->m_castInProgress.has_value())
			combat->m_castInProgress = npc->m_castInProgress.value();
		else
			combat->m_castInProgress.reset();

		if (aEntityState == EntityState::ID_DEAD || aEntityState == EntityState::ID_DESPAWNING)
			position->m_block = false;
		else
			position->m_block = true;
	}

}

