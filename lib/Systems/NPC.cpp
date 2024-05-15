#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/Lootable.h>
#include <tpublic/Components/NPC.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/Sprite.h>
#include <tpublic/Components/Tag.h>
#include <tpublic/Components/ThreatTarget.h>

#include <tpublic/Data/Faction.h>
#include <tpublic/Data/LootTable.h>
#include <tpublic/Data/NPCBehaviorState.h>

#include <tpublic/Systems/NPC.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Haste.h>
#include <tpublic/Helpers.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IGroupRoundRobin.h>
#include <tpublic/IMoveRequestQueue.h>
#include <tpublic/ItemInstanceData.h>
#include <tpublic/IThreatEventQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/ISystemEventQueue.h>
#include <tpublic/LootGenerator.h>
#include <tpublic/Manifest.h>
#include <tpublic/MapData.h>

namespace tpublic::Systems
{

	NPC::NPC(
		const SystemData*		aData)
		: SystemBase(aData)
	{
		RequireComponent<Components::Auras>();
		RequireComponent<Components::CombatPublic>();
		RequireComponent<Components::Lootable>();
		RequireComponent<Components::NPC>();
		RequireComponent<Components::Position>();
		RequireComponent<Components::Tag>();
		RequireComponent<Components::ThreatTarget>();
	}
	
	NPC::~NPC()
	{

	}

	//--------------------------------------------------------------------------------------------

	void		
	NPC::Init(
		uint32_t				/*aEntityId*/,
		uint32_t				/*aEntityInstanceId*/,
		EntityState::Id			/*aEntityState*/,
		ComponentBase**			aComponents,
		int32_t					/*aTick*/)
	{		
		Components::NPC* npc = GetComponent<Components::NPC>(aComponents);
	
		// Initialize resources		
		{
			Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);

			for (const Components::NPC::ResourceEntry& resource : npc->m_resources.m_entries)
			{
				combatPublic->AddResourceMax(resource.m_id, resource.m_max);

				const Resource::Info* info = Resource::GetInfo((Resource::Id)resource.m_id);
				if (info->m_flags & Resource::FLAG_DEFAULT_TO_MAX)
					combatPublic->SetResourceToMax(resource.m_id);
			}

			combatPublic->SetDirty();
		}

		// Remember spawn position
		{
			const Components::Position* position = GetComponent<Components::Position>(aComponents);
			npc->m_spawnPosition = position->m_position;
		}
	}

	EntityState::Id
	NPC::UpdatePrivate(
		uint32_t				aEntityId,
		uint32_t				aEntityInstanceId,
		EntityState::Id			aEntityState,
		int32_t					aTicksInState,
		ComponentBase**			aComponents,
		Context*				aContext) 
	{		
		if(aEntityState == EntityState::ID_SPAWNING)
			return aTicksInState < SPAWN_TICKS ? EntityState::CONTINUE : EntityState::ID_DEFAULT;

		if (aEntityState == EntityState::ID_DESPAWNING)
			return aTicksInState < DESPAWN_TICKS ? EntityState::CONTINUE : EntityState::ID_DESPAWNED;

		const Components::CombatPublic* combat = GetComponent<Components::CombatPublic>(aComponents);
		Components::NPC* npc = GetComponent<Components::NPC>(aComponents);
		Components::Position* position = GetComponent<Components::Position>(aComponents);
		Components::ThreatTarget* threat = GetComponent<Components::ThreatTarget>(aComponents);
		const Components::Auras* auras = GetComponent<Components::Auras>(aComponents);
		const Components::NPC::StateEntry* state = npc->GetState(aEntityState);

		const Data::Faction* faction = GetManifest()->GetById<Data::Faction>(combat->m_factionId);

		if (aEntityState != EntityState::ID_DEAD && combat->GetResource(Resource::ID_HEALTH) == 0 && !auras->HasEffect(AuraEffect::ID_IMMORTALITY, NULL))
		{
			const Components::Tag* tag = GetComponent<Components::Tag>(aComponents);
			if(tag->m_playerTag.IsSet())
			{
				if(tag->m_playerTag.IsGroup())
					aContext->m_eventQueue->EventQueueGroupKillXP(tag->m_playerTag.GetGroupId(), combat->m_level, aEntityId);
				else if(tag->m_playerTag.IsCharacter())
					aContext->m_eventQueue->EventQueueIndividualKillXP(tag->m_playerTag.GetCharacterId(), tag->m_playerTag.GetCharacterLevel(), combat->m_level, aEntityId);

				Components::Lootable* lootable = GetComponent<Components::Lootable>(aComponents);
				lootable->m_playerTag = tag->m_playerTag;
				lootable->SetDirty();

				if(lootable->m_playerTag.IsSet())
				{
					const MapData* mapData = aContext->m_worldView->WorldViewGetMapData();
					if(lootable->m_lootTableId != 0 || mapData->m_mapInfo.m_mapLootTableIds.size() > 0)
						aContext->m_eventQueue->EventQueueGenerateLoot(aEntityInstanceId);
				}
			}

			npc->m_castInProgress.reset();

			return EntityState::ID_DEAD;
		}

		if(aEntityState == EntityState::ID_DEAD && !threat->m_table.IsEmpty())
			aContext->m_eventQueue->EventQueueThreatClear(aEntityInstanceId);

		npc->m_cooldowns.Update(aContext->m_tick);

		std::vector<uint32_t> threatRemovedEntityInstanceIds;
		threat->m_table.Update(aContext->m_tick, threatRemovedEntityInstanceIds);

		for(uint32_t threatRemovedInstanceId : threatRemovedEntityInstanceIds)
			aContext->m_eventQueue->EventQueueThreat(threatRemovedInstanceId, aEntityInstanceId, INT32_MIN);

		if(combat->m_interrupt.has_value() && npc->m_castInProgress.has_value())
		{
			if(combat->m_interrupt->m_cooldownId != 0)
				npc->m_cooldowns.AddCooldown(combat->m_interrupt->m_cooldownId, combat->m_interrupt->m_ticks, aContext->m_tick);

			npc->m_castInProgress.reset();
		}

		EntityState::Id returnValue = EntityState::CONTINUE;

		if(aEntityState == EntityState::ID_DEFAULT || aEntityState == EntityState::ID_IN_COMBAT)
		{
			if(npc->m_castInProgress && aContext->m_tick >= npc->m_castInProgress->m_end)
			{				
				aContext->m_eventQueue->EventQueueAbility(
					aEntityInstanceId, 
					npc->m_castInProgress->m_targetEntityInstanceId,
					npc->m_castInProgress->m_aoeTarget,
					GetManifest()->GetById<Data::Ability>(npc->m_castInProgress->m_abilityId),
					ItemInstanceReference(),
					NULL);
				npc->m_castInProgress.reset();
			}

			const EntityInstance* topThreatEntity = NULL;
			if(!threat->m_table.IsEmpty())
			{
				topThreatEntity = aContext->m_worldView->WorldViewSingleEntityInstance(threat->m_table.GetTop()->m_entityInstanceId);
				if(topThreatEntity != NULL && topThreatEntity->GetState() == EntityState::ID_DEAD)
					topThreatEntity = NULL;
			}

			if(aContext->m_tick - threat->m_lastPingTick >= Components::ThreatTarget::PING_INTERVAL_TICKS)
			{
				IWorldView::EntityQuery entityQuery;
				entityQuery.m_position = position->m_position;
				entityQuery.m_maxDistance = 4;
				entityQuery.m_flags = IWorldView::EntityQuery::FLAG_LINE_OF_SIGHT;

				aContext->m_worldView->WorldViewQueryEntityInstances(entityQuery, [&](
					const EntityInstance* aEntity)
				{
					if(aEntity->GetState() != EntityState::ID_DEAD)
					{
						if (aEntity->IsPlayer() && (!faction->IsNeutralOrFriendly() || combat->m_targetEntityInstanceId == aEntity->GetEntityInstanceId()))
						{
							aContext->m_eventQueue->EventQueueThreat(aEntity->GetEntityInstanceId(), aEntityInstanceId, 0);
						}
						else if(topThreatEntity != NULL && aEntity->GetState() != EntityState::ID_IN_COMBAT && !faction->IsNeutralOrFriendly())
						{
							const tpublic::Components::CombatPublic* nearbyNonPlayerCombatPublic = aEntity->GetComponent<tpublic::Components::CombatPublic>();
							if(nearbyNonPlayerCombatPublic != NULL && nearbyNonPlayerCombatPublic->m_factionId == combat->m_factionId)
								aContext->m_eventQueue->EventQueueThreat(topThreatEntity->GetEntityInstanceId(), aEntity->GetEntityInstanceId(), 0);
						}
					}

					return false;
				});

				threat->m_lastPingTick = aContext->m_tick;
			}
		}

		if(aEntityState != EntityState::ID_IN_COMBAT && aEntityState != EntityState::ID_EVADING)
			position->m_lastMoveTick = aContext->m_tick;
		
		switch(aEntityState)
		{
		case EntityState::ID_DEAD:
			{
				const Components::Lootable* lootable = GetComponent<Components::Lootable>(aComponents);
				bool hasLoot = lootable->HasLoot();
				if((hasLoot && aTicksInState >= npc->m_despawnTime.m_ticksWithLoot) || 
					(!hasLoot && aTicksInState >= npc->m_despawnTime.m_ticksWithoutLoot))
				{
					returnValue = EntityState::ID_DESPAWNING;
				}
			}
			break;

		case EntityState::ID_DEFAULT:
			if (!threat->m_table.IsEmpty())
			{
				threat->m_lastPingTick = 0; // Force a threat ping (will cause nearby allies to join)

				npc->m_anchorPosition = position->m_position; // Remember this position, this is where we'll go back to if we evade

				npc->m_targetEntityInstanceId = threat->m_table.GetTop()->m_entityInstanceId;
				npc->m_npcBehaviorState = NULL;
				npc->m_moveCooldownUntilTick = 0;
				npc->SetDirty();

				returnValue = EntityState::ID_IN_COMBAT;
			}
			else if(npc->m_npcBehaviorState == NULL && npc->m_defaultBehaviorState != 0)
			{
				npc->m_npcBehaviorState = GetManifest()->GetById<Data::NPCBehaviorState>(npc->m_defaultBehaviorState);
				npc->m_npcBehaviorStateTick = aContext->m_tick;
			}
			else if(npc->m_npcBehaviorState != NULL)
			{
				if(npc->m_npcBehaviorState->m_maxTicks != 0 && (aContext->m_tick - npc->m_npcBehaviorStateTick) > npc->m_npcBehaviorState->m_maxTicks)
				{
					// FIXME: do something
				}
				else
				{
					switch(npc->m_npcBehaviorState->m_behavior)
					{
					case NPCBehavior::ID_WANDERING:
						if (npc->m_moveCooldownUntilTick < aContext->m_tick)
						{
							static const Vec2 WANDER_DIRECTIONS[] = 
							{
								{ 1, 0 },
								{ 0, 1 },
								{ -1, 0 },
								{ 0, -1 }
							};

							size_t randomIndex = aContext->m_random->operator()() % 4;
							const Vec2& direction = WANDER_DIRECTIONS[randomIndex];
							Vec2 newPosition = { direction.m_x + position->m_position.m_x, direction.m_y + position->m_position.m_y };
							Vec2 spawnDirection = { newPosition.m_x - npc->m_spawnPosition.m_x, newPosition.m_y - npc->m_spawnPosition.m_y };
							int32_t spawnSquaredDistance = spawnDirection.m_x * spawnDirection.m_x + spawnDirection.m_y * spawnDirection.m_y;

							if(spawnSquaredDistance <= (int32_t)(npc->m_npcBehaviorState->m_maxRange * npc->m_npcBehaviorState->m_maxRange))
							{
								IEventQueue::EventQueueMoveRequest moveRequest;
								moveRequest.AddToPriorityList(direction);
								moveRequest.m_type = IEventQueue::EventQueueMoveRequest::TYPE_SIMPLE;
								moveRequest.m_entityInstanceId = aEntityInstanceId;

								aContext->m_eventQueue->EventQueueMove(moveRequest);
							}

							npc->m_moveCooldownUntilTick = aContext->m_tick + 10 + (uint32_t)aContext->m_random->operator()() % 10;
						}
						break;

					default:
						break;
					}
				}
			}
			break;

		case EntityState::ID_IN_COMBAT:
			{
				if (threat->m_table.IsEmpty())
				{
					// Empty threat table 
					npc->m_targetEntityInstanceId = 0;
					npc->m_castInProgress.reset();
					npc->m_npcMovement.Reset(aContext->m_tick);				

					Components::Tag* tag = GetComponent<Components::Tag>(aComponents);
					tag->m_playerTag.Clear();
					tag->SetDirty();

					position->m_lastMoveTick = aContext->m_tick;

					returnValue = EntityState::ID_EVADING;
				}
				else if(!auras->HasEffect(AuraEffect::ID_STUN, NULL))
				{
					uint32_t topThreatEntityInstanceId;

					if(!auras->HasEffect(AuraEffect::ID_TAUNT, &topThreatEntityInstanceId))
						topThreatEntityInstanceId = threat->m_table.GetTop()->m_entityInstanceId;

					npc->m_targetEntityInstanceId = topThreatEntityInstanceId;
					
					const EntityInstance* target = aContext->m_worldView->WorldViewSingleEntityInstance(npc->m_targetEntityInstanceId);
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
							const Data::Ability* ability = GetManifest()->GetById<tpublic::Data::Ability>(abilityEntry.m_abilityId);

							if (distanceSquared > (int32_t)(ability->m_range * ability->m_range))
								continue;
								
							if(npc->m_cooldowns.IsAbilityOnCooldown(ability))
								continue;

							if(!combat->HasResourcesForAbility(ability, NULL, combat->GetResourceMax(Resource::ID_MANA)))
								continue;

							if(!aContext->m_worldView->WorldViewLineOfSight(targetPosition->m_position, position->m_position))
								continue;

							if (abilityEntry.m_useProbability == UINT32_MAX || (*aContext->m_random)() < abilityEntry.m_useProbability)
							{
								useAbility = ability;
								break;
							}
						}

						if (useAbility != NULL)
						{
							position->m_lastMoveTick = aContext->m_tick;
							npc->m_npcMovement.Reset(aContext->m_tick);

							float cooldownModifier = 0.0f;
							if(useAbility->IsAttack() && useAbility->IsMelee())
								cooldownModifier = auras->GetAttackHaste(GetManifest());

							npc->m_cooldowns.AddAbility(GetManifest(), useAbility, aContext->m_tick, cooldownModifier);

							if(useAbility->m_castTime > 0)
							{	
								int32_t castTime = useAbility->m_castTime;
								if(useAbility->IsSpell())
									castTime = Haste::CalculateCastTime(castTime, auras->GetSpellHaste(GetManifest()));

								CastInProgress cast;
								cast.m_abilityId = useAbility->m_id;
								cast.m_targetEntityInstanceId = target->GetEntityInstanceId();
								cast.m_start = aContext->m_tick;
								cast.m_end = cast.m_start + castTime;
								npc->m_castInProgress = cast;
							}
							else
							{
								aContext->m_eventQueue->EventQueueAbility(aEntityInstanceId, target->GetEntityInstanceId(), Vec2(), useAbility, ItemInstanceReference(), NULL);
							}
						}
						else if(distanceSquared > 1 && !auras->HasEffect(AuraEffect::ID_IMMOBILIZE, NULL))
						{
							const MoveSpeed::Info* moveSpeedInfo = MoveSpeed::GetInfo(combat->m_moveSpeed);
							if(npc->m_moveCooldownUntilTick + moveSpeedInfo->m_tickBias < aContext->m_tick)
							{
								if (npc->m_npcMovement.ShouldResetIfLOS(aContext->m_tick))
								{
									if (aContext->m_worldView->WorldViewLineOfSight(position->m_position, targetPosition->m_position))
										npc->m_npcMovement.Reset(aContext->m_tick);
								}

								IEventQueue::EventQueueMoveRequest moveRequest;
								if (npc->m_npcMovement.GetMoveRequest(
									aContext->m_worldView->WorldViewGetMapData()->m_mapPathData.get(),
									position->m_position,
									targetPosition->m_position,
									aContext->m_tick,
									position->m_lastMoveTick,
									moveRequest))
								{
									moveRequest.m_entityInstanceId = aEntityInstanceId;
									moveRequest.m_canMoveOnAllNonViewBlockingTiles = npc->m_canMoveOnAllNonViewBlockingTiles;

									aContext->m_eventQueue->EventQueueMove(moveRequest);

									npc->m_moveCooldownUntilTick = aContext->m_tick + 2;
								}
							}
						}
					}
				}
			}
			break;

		case EntityState::ID_EVADING:
			if(position->m_position == npc->m_anchorPosition)
			{
				returnValue = EntityState::ID_DEFAULT;
			}
			else if(npc->m_moveCooldownUntilTick < aContext->m_tick)
			{
				// FIXME: since (future me: what?)
				IEventQueue::EventQueueMoveRequest moveRequest;
				if (!npc->m_npcMovement.GetMoveRequest(aContext->m_worldView->WorldViewGetMapData()->m_mapPathData.get(), position->m_position, npc->m_anchorPosition, aContext->m_tick, position->m_lastMoveTick, moveRequest))
				{
					// Can't seem to move, teleport all the way back to anchor
					moveRequest.m_type = IEventQueue::EventQueueMoveRequest::TYPE_SIMPLE;
					moveRequest.AddToPriorityList(npc->m_anchorPosition - position->m_position);
				}

				moveRequest.m_entityInstanceId = aEntityInstanceId;
				moveRequest.m_canMoveOnAllNonViewBlockingTiles = npc->m_canMoveOnAllNonViewBlockingTiles;

				aContext->m_eventQueue->EventQueueMove(moveRequest);

				npc->m_moveCooldownUntilTick = aContext->m_tick + 2;
			}
			break;

		default:
			break;
		}

		return returnValue;
	}

	void			
	NPC::UpdatePublic(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			/*aContext*/) 
	{
		Components::CombatPublic* combat = GetComponent<Components::CombatPublic>(aComponents);
		const Components::NPC* npc = GetComponent<Components::NPC>(aComponents);
		Components::Position* position = GetComponent<Components::Position>(aComponents);

		if(combat->m_targetEntityInstanceId != npc->m_targetEntityInstanceId)
		{
			combat->m_targetEntityInstanceId = npc->m_targetEntityInstanceId;
			combat->SetDirty();
		}

		if(npc->m_castInProgress.has_value())
		{
			if(combat->m_castInProgress != npc->m_castInProgress.value())
			{
				combat->m_castInProgress = npc->m_castInProgress.value();
				combat->SetDirty();
			}
		}
		else 
		{
			if (combat->m_castInProgress.has_value())
			{
				combat->m_castInProgress.reset();
				combat->SetDirty();
			}
		}

		bool block = 
			aEntityState != EntityState::ID_DEAD && 
			aEntityState != EntityState::ID_EVADING && 
			aEntityState != EntityState::ID_DESPAWNING && 
			aEntityState != EntityState::ID_DESPAWNED;

		if(position->IsBlocking() != block)
		{
			if(block)
				position->SetBlocking();
			else
				position->ClearBlocking();
			position->SetDirty();
		}

		combat->m_interrupt.reset();
	}

}

