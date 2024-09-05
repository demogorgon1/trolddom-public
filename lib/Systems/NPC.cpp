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
#include <tpublic/Manifest.h>
#include <tpublic/MapData.h>
#include <tpublic/Requirements.h>

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

		// Remember spawn position and set position size flag if needed
		{
			Components::Position* position = GetComponent<Components::Position>(aComponents);
			npc->m_spawnPosition = position->m_position;

			if(npc->m_large)
			{
				position->SetLarge();
				position->SetDirty();

				npc->m_npcMovement.SetDirectOnly(true);
			}
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

		if (state != NULL && state->m_onEnterAbilityId != 0 && aTicksInState == 1)
		{
			const Data::Ability* onEnterAbility = GetManifest()->GetById<Data::Ability>(state->m_onEnterAbilityId);
			aContext->m_eventQueue->EventQueueAbility(aEntityInstanceId, aEntityInstanceId, Vec2(), onEnterAbility, ItemInstanceReference(), NULL);
		}

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

		if(npc->m_encounterId == 0)
		{
			std::vector<uint32_t> threatRemovedEntityInstanceIds;
			threat->m_table.Update(aContext->m_tick, threatRemovedEntityInstanceIds);

			for (uint32_t threatRemovedInstanceId : threatRemovedEntityInstanceIds)
				aContext->m_eventQueue->EventQueueThreat(threatRemovedInstanceId, aEntityInstanceId, INT32_MIN, aContext->m_tick);
		}
		else if(npc->m_inactiveEncounterDespawn && !aContext->m_worldView->WorldViewIsEncounterActive(npc->m_encounterId))
		{
			return EntityState::ID_DESPAWNING;
		}

		if(combat->m_interrupt.has_value() && npc->m_castInProgress.has_value())
		{
			if(combat->m_interrupt->m_cooldownId != 0)
				npc->m_cooldowns.AddCooldown(combat->m_interrupt->m_cooldownId, combat->m_interrupt->m_ticks, aContext->m_tick);

			if(npc->m_castInProgress->m_channeling)
				aContext->m_eventQueue->EventQueueCancelChanneling(aEntityInstanceId, npc->m_castInProgress->m_targetEntityInstanceId, npc->m_castInProgress->m_abilityId, npc->m_castInProgress->m_start);

			npc->m_castInProgress.reset();
		}

		{
			CastInProgress channeling;
			if(aContext->m_worldView->WorldViewGetChanneling(aEntityInstanceId, channeling))
			{
				npc->m_castInProgress = channeling;
			}
		}

		if (npc->m_castInProgress && aContext->m_tick >= npc->m_castInProgress->m_end)
		{
			if (!npc->m_castInProgress->m_channeling)
			{
				aContext->m_eventQueue->EventQueueAbility(
					aEntityInstanceId,
					npc->m_castInProgress->m_targetEntityInstanceId,
					npc->m_castInProgress->m_aoeTarget,
					GetManifest()->GetById<Data::Ability>(npc->m_castInProgress->m_abilityId),
					ItemInstanceReference(),
					NULL);
			}
			npc->m_castInProgress.reset();
		}

		EntityState::Id returnValue = EntityState::CONTINUE;

		if(aEntityState == EntityState::ID_DEFAULT || aEntityState == EntityState::ID_IN_COMBAT)
		{
			const EntityInstance* topThreatEntity = NULL;
			const ThreatTable::Entry* topThreatEntry = NULL;

			if(!threat->m_table.IsEmpty())
			{
				topThreatEntry = threat->m_table.GetTop();
				topThreatEntity = aContext->m_worldView->WorldViewSingleEntityInstance(topThreatEntry->m_entityInstanceId);
				if(topThreatEntity != NULL && topThreatEntity->GetState() == EntityState::ID_DEAD)
					topThreatEntity = NULL;
			}

			bool passive = (aEntityState == EntityState::ID_DEFAULT) && aTicksInState < 15;

			if(aContext->m_tick - threat->m_lastPingTick >= Components::ThreatTarget::PING_INTERVAL_TICKS && !passive)
			{
				IWorldView::EntityQuery entityQuery;
				entityQuery.m_position = position->m_position;
				entityQuery.m_maxDistance = GetManifest()->m_npcMetrics.GetMaxAggroRange();
				entityQuery.m_flags = IWorldView::EntityQuery::FLAG_LINE_OF_SIGHT;

				aContext->m_worldView->WorldViewQueryEntityInstances(entityQuery, [&](
					const EntityInstance* aEntity,
					int32_t				  aDistanceSquared)
				{
					if(aEntity->GetState() != EntityState::ID_DEAD)
					{
						if (aEntity->IsPlayer() && (!faction->IsNeutralOrFriendly() || combat->m_targetEntityInstanceId == aEntity->GetEntityInstanceId()))
						{
							const tpublic::Components::CombatPublic* playerCombatPublic = aEntity->GetComponent<tpublic::Components::CombatPublic>();
							int32_t aggroRange = GetManifest()->m_npcMetrics.GetAggroRangeForLevelDifference((int32_t)combat->m_level, (int32_t)playerCombatPublic->m_level);
							if(aDistanceSquared <= aggroRange * aggroRange)
								aContext->m_eventQueue->EventQueueThreat(aEntity->GetEntityInstanceId(), aEntityInstanceId, 0, aContext->m_tick);
						}
						else if(topThreatEntity != NULL && aEntity->GetState() != EntityState::ID_IN_COMBAT && !faction->IsNeutralOrFriendly())
						{
							const tpublic::Components::CombatPublic* nearbyNonPlayerCombatPublic = aEntity->GetComponent<tpublic::Components::CombatPublic>();
							int32_t aggroAssistRange = GetManifest()->m_npcMetrics.m_aggroAssistRange;
							if(nearbyNonPlayerCombatPublic != NULL && nearbyNonPlayerCombatPublic->m_factionId == combat->m_factionId && aDistanceSquared <= aggroAssistRange * aggroAssistRange)
								aContext->m_eventQueue->EventQueueThreat(topThreatEntity->GetEntityInstanceId(), aEntity->GetEntityInstanceId(), 0, topThreatEntry->m_tick);
						}
					}

					return false;
				});

				// Add a little randomness to when the ping is done to spread out server load
				threat->m_lastPingTick = aContext->m_tick + (int32_t)((*aContext->m_random)() % 2);
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

				if(npc->m_targetEntityInstanceId != 0)
					npc->m_targetEntityInstanceId = 0;
			}
			break;

		case EntityState::ID_DEFAULT:
			if(npc->m_spawnWithTarget.has_value())
			{
				aContext->m_eventQueue->EventQueueThreat(npc->m_spawnWithTarget->m_entityInstanceId, aEntityInstanceId, npc->m_spawnWithTarget->m_threat, aContext->m_tick);
				npc->m_spawnWithTarget.reset();
			}
			else if (!threat->m_table.IsEmpty())
			{
				threat->m_lastPingTick = 0; // Force a threat ping (will cause nearby allies to join)

				npc->m_anchorPosition = position->m_position; // Remember this position, this is where we'll go back to if we evade

				npc->m_targetEntityInstanceId = threat->m_table.GetTop()->m_entityInstanceId;
				npc->m_npcBehaviorState = NULL;
				npc->m_moveCooldownUntilTick = 0;
				npc->m_lastAttackTick = aContext->m_tick;
				npc->SetDirty();

				const Components::NPC::StateEntry* inCombatState = npc->GetState(EntityState::ID_IN_COMBAT);
				if(inCombatState != NULL && inCombatState->m_barks.size() > 0)
				{
					const Chat* bark = Helpers::RandomItemPointer(*aContext->m_random, inCombatState->m_barks);
					assert(bark != NULL);
					aContext->m_eventQueue->EventQueueChat(aEntityInstanceId, *bark);
				}				

				if(npc->m_encounterId != 0)
					aContext->m_eventQueue->EventQueueStartEncounter(aEntityId, aEntityInstanceId, npc->m_encounterId);

				returnValue = EntityState::ID_IN_COMBAT;
			}
			else if(npc->m_despawnTime.m_ticksOutOfCombat != 0 && aTicksInState > npc->m_despawnTime.m_ticksOutOfCombat)
			{
				returnValue = EntityState::ID_DESPAWNING;
			}
			else if(npc->m_npcBehaviorState == NULL && npc->m_defaultBehaviorStateId != 0)
			{
				npc->m_npcBehaviorState = GetManifest()->GetById<Data::NPCBehaviorState>(npc->m_defaultBehaviorStateId);
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

					case NPCBehavior::ID_USE_ABILITY:
						if (!npc->m_castInProgress.has_value() && !auras->HasEffect(AuraEffect::ID_STUN, NULL))
						{
							for(const Components::NPC::AbilityEntry& abilityEntry : state->m_abilities)
							{
								const Data::Ability* ability = GetManifest()->GetById<Data::Ability>(abilityEntry.m_abilityId);
								if(npc->m_cooldowns.IsAbilityOnCooldown(ability))
									continue;

								if (!combat->HasResourcesForAbility(ability, NULL, combat->GetResourceMax(Resource::ID_MANA)))
									continue;

								if (abilityEntry.m_useProbability == UINT32_MAX || (*aContext->m_random)() < abilityEntry.m_useProbability)
								{
									uint32_t targetEntityInstanceId = 0;

									IWorldView::EntityQuery entityQuery;
									entityQuery.m_position = position->m_position;
									entityQuery.m_maxDistance = ability->m_range;
									entityQuery.m_entityIds = &abilityEntry.m_targetEntityIds;
									entityQuery.m_flags = IWorldView::EntityQuery::FLAG_LINE_OF_SIGHT;

									aContext->m_worldView->WorldViewQueryEntityInstances(entityQuery, [&](
										const EntityInstance* aEntityInstance,
										int32_t /*aDistanceSquared*/) -> bool
										{
											// FIXME: we might need to check if this is compatible with this ability
											targetEntityInstanceId = aEntityInstance->GetEntityInstanceId();
											return true;
										});

									if (targetEntityInstanceId != 0)
									{
										npc->m_cooldowns.AddAbility(GetManifest(), ability, aContext->m_tick, 0.0f); // FIXME: cooldown modifier from haste?

										if (ability->m_castTime > 0)
										{
											int32_t castTime = ability->m_castTime;
											if (ability->IsSpell())
												castTime = Haste::CalculateCastTime(castTime, auras->GetSpellHaste(GetManifest()));

											CastInProgress cast;
											cast.m_abilityId = ability->m_id;
											cast.m_targetEntityInstanceId = targetEntityInstanceId;
											cast.m_start = aContext->m_tick;
											cast.m_end = cast.m_start + castTime;
											npc->m_castInProgress = cast;
										}
										else
										{
											aContext->m_eventQueue->EventQueueAbility(aEntityInstanceId, targetEntityInstanceId, Vec2(), ability, ItemInstanceReference(), NULL);
										}

										break;
									}
								}
							}
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
					{
						const ThreatTable::Entry* topThreatEntry = threat->m_table.GetTop();
						topThreatEntityInstanceId = topThreatEntry->m_entityInstanceId;
					}

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
						int32_t distanceSquared = Helpers::CalculateDistanceSquared(targetPosition, position);

						const Data::Ability* useAbility = NULL;

						for (const Components::NPC::AbilityEntry& abilityEntry : state->m_abilities)
						{
							const Data::Ability* ability = GetManifest()->GetById<tpublic::Data::Ability>(abilityEntry.m_abilityId);

							if(abilityEntry.m_requirements.size() > 0)
							{
								const EntityInstance* self = aContext->m_worldView->WorldViewSingleEntityInstance(aEntityInstanceId);
								if(self != NULL && !Requirements::CheckList(GetManifest(), abilityEntry.m_requirements, self, target))
									continue;
							}

							if (abilityEntry.m_targetType == Components::NPC::AbilityEntry::TARGET_TYPE_DEFAULT)
							{
								if (distanceSquared > (int32_t)(ability->m_range * ability->m_range))
									continue;

								if (distanceSquared < (int32_t)(ability->m_minRange * ability->m_minRange))
									continue;

								if (npc->m_cooldowns.IsAbilityOnCooldown(ability))
									continue;

								if (!combat->HasResourcesForAbility(ability, NULL, combat->GetResourceMax(Resource::ID_MANA)))
									continue;

								if(!aContext->m_worldView->WorldViewLineOfSight(targetPosition->m_position, position->m_position))
									continue;

								if (abilityEntry.m_useProbability == UINT32_MAX || (*aContext->m_random)() < abilityEntry.m_useProbability)
								{
									useAbility = ability;
									break;
								}
							}
							else if (abilityEntry.m_targetType == Components::NPC::AbilityEntry::TARGET_TYPE_LOW_HEALTH_FRIEND_OR_SELF)
							{
								if (npc->m_cooldowns.IsAbilityOnCooldown(ability))
									continue;

								if (!combat->HasResourcesForAbility(ability, NULL, combat->GetResourceMax(Resource::ID_MANA)))
									continue;

								IWorldView::EntityQuery entityQuery;
								entityQuery.m_position = position->m_position;
								entityQuery.m_maxDistance = ability->m_range;
								entityQuery.m_flags = IWorldView::EntityQuery::FLAG_LINE_OF_SIGHT;

								std::vector<const EntityInstance*> possibleTargets;
								aContext->m_worldView->WorldViewQueryEntityInstances(entityQuery, [&](
									const EntityInstance* aEntity,
									int32_t				  aDistanceSquared)
								{
									if (aEntity->GetState() == EntityState::ID_IN_COMBAT && aDistanceSquared >= (int32_t)(ability->m_minRange * ability->m_minRange))
									{
										const Components::CombatPublic* otherCombatPublic = aEntity->GetComponent<Components::CombatPublic>();
										if(otherCombatPublic != NULL && aEntity->HasComponent<Components::NPC>())
										{
											if(otherCombatPublic->m_factionId == combat->m_factionId && otherCombatPublic->GetResourcePercentage(Resource::ID_HEALTH) < 80)
												possibleTargets.push_back(aEntity);
										}
									}
									return false;
								});

								if (possibleTargets.size() > 0)
								{
									const EntityInstance* selectedTarget = possibleTargets[Helpers::RandomInRange<size_t>(*aContext->m_random, 0, possibleTargets.size() - 1)];

									useAbility = ability;
									npc->m_targetEntityInstanceId = selectedTarget->GetEntityInstanceId();
									break;
								}
							}
							else if(abilityEntry.m_targetType == Components::NPC::AbilityEntry::TARGET_TYPE_RANDOM_PLAYER)
							{
								if (npc->m_cooldowns.IsAbilityOnCooldown(ability))
									continue;

								if (!combat->HasResourcesForAbility(ability, NULL, combat->GetResourceMax(Resource::ID_MANA)))
									continue;

								IWorldView::EntityQuery entityQuery;
								entityQuery.m_position = position->m_position;
								entityQuery.m_maxDistance = ability->m_range;
								entityQuery.m_flags = IWorldView::EntityQuery::FLAG_LINE_OF_SIGHT;

								std::vector<const EntityInstance*> possibleTargets;
								aContext->m_worldView->WorldViewQueryEntityInstances(entityQuery, [&](
									const EntityInstance* aEntity,
									int32_t				  aDistanceSquared)
								{
									if (aEntity->GetState() != EntityState::ID_DEAD && aEntity->IsPlayer() && aDistanceSquared >= (int32_t)(ability->m_minRange * ability->m_minRange))
										possibleTargets.push_back(aEntity);
									return false;
								});


								if(possibleTargets.size() > 0)
								{									
									const EntityInstance* selectedTarget = possibleTargets[Helpers::RandomInRange<size_t>(*aContext->m_random, 0, possibleTargets.size() - 1)];

									useAbility = ability;
									npc->m_targetEntityInstanceId = selectedTarget->GetEntityInstanceId();
									break;
								}
							}
							else if (abilityEntry.m_targetType == Components::NPC::AbilityEntry::TARGET_TYPE_SELF)
							{
								if (npc->m_cooldowns.IsAbilityOnCooldown(ability))
									continue;

								if (!combat->HasResourcesForAbility(ability, NULL, combat->GetResourceMax(Resource::ID_MANA)))
									continue;

								useAbility = ability;
								npc->m_targetEntityInstanceId = aEntityInstanceId;
								break;
							}
						}

						if (useAbility != NULL)
						{
							position->m_lastMoveTick = aContext->m_tick;
							npc->m_npcMovement.Reset(aContext->m_tick);

							if(useAbility->IsOffensive())
								npc->m_lastAttackTick = aContext->m_tick;

							aContext->m_eventQueue->EventQueueThreat(npc->m_targetEntityInstanceId, aEntityInstanceId, 1, aContext->m_tick);

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
								cast.m_targetEntityInstanceId = npc->m_targetEntityInstanceId;
								cast.m_start = aContext->m_tick;
								cast.m_end = cast.m_start + castTime;
								npc->m_castInProgress = cast;
							}
							else
							{
								aContext->m_eventQueue->EventQueueAbility(aEntityInstanceId, npc->m_targetEntityInstanceId, Vec2(), useAbility, ItemInstanceReference(), NULL);
							}
						}
						else if(distanceSquared > 1 && !auras->HasEffect(AuraEffect::ID_IMMOBILIZE, NULL))
						{
							int32_t ticksSinceLastAttack = aContext->m_tick - npc->m_lastAttackTick;
							if(ticksSinceLastAttack > 80 && npc->m_large) 
							{
								// We're a large NPC and we haven't been able to hurt anyone for some time. We're probably getting cheesed. Evade!
								aContext->m_eventQueue->EventQueueThreatClear(aEntityInstanceId);
							}
							else
							{
								const MoveSpeed::Info* moveSpeedInfo = MoveSpeed::GetInfo(combat->m_moveSpeed);
								if (npc->m_moveCooldownUntilTick + moveSpeedInfo->m_tickBias < aContext->m_tick)
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
									else
									{
										// Seems like we're stuck chasing the top threat target. Reduce threat on that one.
										position->m_lastMoveTick = aContext->m_tick;
										npc->m_npcMovement.Reset(aContext->m_tick);

										aContext->m_eventQueue->EventQueueThreat(npc->m_targetEntityInstanceId, aEntityInstanceId, 0, 0, 0.5f);
									}
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
				npc->m_restoreResources = true;

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
		Components::NPC* npc = GetComponent<Components::NPC>(aComponents);
		Components::Position* position = GetComponent<Components::Position>(aComponents);

		if(npc->m_restoreResources)
		{
			combat->SetDefaultResources();
			combat->SetDirty();

			npc->m_restoreResources = false;
		}

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

		bool block = npc->m_blocking && 
			(aEntityState != EntityState::ID_DEAD && 
			 aEntityState != EntityState::ID_EVADING && 
			 aEntityState != EntityState::ID_DESPAWNING && 
			 aEntityState != EntityState::ID_DESPAWNED);

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

