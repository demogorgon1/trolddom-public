#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/DisplayName.h>
#include <tpublic/Components/KillContribution.h>
#include <tpublic/Components/Lootable.h>
#include <tpublic/Components/NPC.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/Sprite.h>
#include <tpublic/Components/Tag.h>
#include <tpublic/Components/ThreatSource.h>
#include <tpublic/Components/ThreatTarget.h>
#include <tpublic/Components/VisibleAuras.h>

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/Faction.h>
#include <tpublic/Data/LootTable.h>
#include <tpublic/Data/NPCBehaviorState.h>
#include <tpublic/Data/Route.h>

#include <tpublic/Systems/NPC.h>

#include <tpublic/ApplyNPCMetrics.h>
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
#include <tpublic/MapRouteData.h>
#include <tpublic/NPCMetrics.h>
#include <tpublic/RealmModifierList.h>
#include <tpublic/Requirements.h>
#include <tpublic/StealthUtils.h>
#include <tpublic/WorldInfoMap.h>
#include <tpublic/XPMetrics.h>

namespace tpublic::Systems
{

	namespace
	{
		bool
		_IsInMeleeRange(
			const Components::Position* aPosition,
			const EntityInstance*		aEntityInstance)
		{
			if(aEntityInstance == NULL)
				return false;

			const Components::Position* otherPosition = aEntityInstance->GetComponent<Components::Position>();
			if(otherPosition == NULL)
				return false;

			return Helpers::CalculateDistanceSquared(aPosition, otherPosition) <= 1;
		}
	}

	//--------------------------------------------------------------------------------------------

	NPC::NPC(
		const SystemData*		aData)
		: SystemBase(aData)
	{
		RequireComponent<Components::Auras>();
		RequireComponent<Components::CombatPublic>();
		RequireComponent<Components::CombatPrivate>();
		RequireComponent<Components::Lootable>();
		RequireComponent<Components::NPC>();
		RequireComponent<Components::Position>();
		RequireComponent<Components::Tag>();
		RequireComponent<Components::ThreatTarget>();
		RequireComponent<Components::VisibleAuras>();
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

		const NPCMetrics::Level* heroicLevel = NULL;
	
		// Initialize resources		
		{
			Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);

			if (GetData()->m_heroic)
			{
				uint32_t newHeroicLevel = GetManifest()->m_xpMetrics->m_maxLevel;
				if(combatPublic->m_level < newHeroicLevel)
					combatPublic->m_level = newHeroicLevel;

				const NPCMetrics* npcMetrics = GetManifest()->m_npcMetrics.get();
				heroicLevel = npcMetrics->GetLevel(combatPublic->m_level);

				npc->m_heroic = true;
			}

			for (const Components::NPC::ResourceEntry& resource : npc->m_resources.m_entries)
			{
				uint32_t resourceMax = resource.m_max;

				if(heroicLevel != NULL)
				{
					resourceMax = heroicLevel->m_baseResource[resource.m_id];

					if((Resource::Id)resource.m_id == Resource::ID_HEALTH)
					{
						if(combatPublic->IsElite())
							resourceMax = (uint32_t)((float)resourceMax * npc->m_heroicHealthMultiplier * heroicLevel->m_eliteResource[resource.m_id]);
						else
							resourceMax = (uint32_t)((float)resourceMax * npc->m_heroicHealthMultiplier);
					}

				}

				combatPublic->AddResourceMax(resource.m_id, resourceMax);

				const Resource::Info* info = Resource::GetInfo((Resource::Id)resource.m_id);
				if (info->m_flags & Resource::FLAG_DEFAULT_TO_MAX)
					combatPublic->SetResourceToMax(resource.m_id);
			}

			combatPublic->SetDirty();
		}

		// If heroic update weapon damage and armor
		if(heroicLevel != NULL)
		{
			Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);

			combatPublic->m_overrideWeaponDamageRangeMin = (uint32_t)((float)heroicLevel->m_baseWeaponDamage.m_min * npc->m_heroicWeaponDamagerMultiplier * heroicLevel->m_eliteWeaponDamage);
			combatPublic->m_overrideWeaponDamageRangeMax = (uint32_t)((float)heroicLevel->m_baseWeaponDamage.m_max * npc->m_heroicWeaponDamagerMultiplier * heroicLevel->m_eliteWeaponDamage);

			combatPublic->m_overrideArmor = heroicLevel->m_baseArmor;
		}

		// Remember spawn position, set position size flag if needed
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

		// Initialize effective route id
		{
			if(npc->m_routeId != 0)
				npc->m_effectiveRouteId = npc->m_routeId;
		}

		// Init "no combat tag" flag 
		if(npc->m_noCombatTag)
		{
			Components::Tag* tag = GetComponent<Components::Tag>(aComponents);
			tag->m_noCombatTag = true;
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
		Components::NPC* npc = GetComponent<Components::NPC>(aComponents);

		if(aEntityState == EntityState::ID_SPAWNING)
		{
			if(aTicksInState < SPAWN_TICKS)
				return EntityState::CONTINUE;

			const Components::NPC::StateEntry* spawningState = npc->GetState(EntityState::ID_SPAWNING);
			if (spawningState != NULL && spawningState->m_barks.size() > 0)
			{
				const Chat* bark = Helpers::RandomItemPointer(*aContext->m_random, spawningState->m_barks);
				assert(bark != NULL);
				aContext->m_eventQueue->EventQueueChat(aEntityInstanceId, *bark);
			}

			return EntityState::ID_DEFAULT;
		}

		if (aEntityState == EntityState::ID_DESPAWNING)
			return aTicksInState < DESPAWN_TICKS ? EntityState::CONTINUE : EntityState::ID_DESPAWNED;

		const Components::CombatPublic* combat = GetComponent<Components::CombatPublic>(aComponents);
		Components::Position* position = GetComponent<Components::Position>(aComponents);
		Components::ThreatTarget* threat = GetComponent<Components::ThreatTarget>(aComponents);
		const Components::Auras* auras = GetComponent<Components::Auras>(aComponents);
		const Components::NPC::StateEntry* state = npc->GetState(aEntityState);

		if (state != NULL && state->m_onEnterAbilityId != 0 && aTicksInState == 1)
		{
			const Data::Ability* onEnterAbility = GetManifest()->GetById<Data::Ability>(state->m_onEnterAbilityId);
			aContext->m_eventQueue->EventQueueAbility({ aEntityInstanceId, 0 }, aEntityInstanceId, Vec2(), onEnterAbility, ItemInstanceReference(), NULL);
		}

		bool despawnOnLeaveState = state != NULL && state->m_despawnOnLeave;	
		uint32_t despawnAfterTicks = state != NULL ? state->m_despawnAfterTicks : 0;

		if(despawnAfterTicks != 0 && (uint32_t)aTicksInState > despawnAfterTicks)
			return EntityState::ID_DESPAWNING;

		if(npc->m_requiredSeasonalEventId != 0 && !aContext->m_worldView->WorldViewIsSeasonalEventActive(npc->m_requiredSeasonalEventId))
			return EntityState::ID_DESPAWNING;

		const Data::Faction* faction = GetManifest()->GetById<Data::Faction>(combat->m_factionId);

		if (npc->m_canDie && aEntityState != EntityState::ID_DEAD && combat->GetResource(Resource::ID_HEALTH) == 0 && !auras->HasEffect(AuraEffect::ID_IMMORTALITY, NULL))
		{
			const Components::Tag* tag = GetComponent<Components::Tag>(aComponents);
			if(tag->m_playerTag.IsSet())
			{
				if(!npc->m_noKillEvent)
				{
					if (tag->m_playerTag.IsGroup())
					{
						std::vector<uint32_t> threatEntityInstanceIds;
						threat->m_table.GetEntityInstanceIds(threatEntityInstanceIds);
						aContext->m_eventQueue->EventQueueGroupKillXP(tag->m_playerTag.GetGroupId(), combat->m_level, aEntityId, combat->m_factionId, threatEntityInstanceIds);
					}
					else if (tag->m_playerTag.IsCharacter())
					{
						aContext->m_eventQueue->EventQueueIndividualKillXP(tag->m_playerTag.GetCharacterId(), tag->m_playerTag.GetCharacterLevel(), combat->m_level, aEntityId, combat->m_factionId);
					}
				}

				Components::Lootable* lootable = GetComponent<Components::Lootable>(aComponents);
				lootable->m_timeStamp = (uint64_t)time(NULL);

				if(lootable->m_anyoneCanLoot)
					lootable->m_playerTag = PlayerTag(PlayerTag::TYPE_ANYONE);
				else
					lootable->m_playerTag = tag->m_playerTag;

				lootable->SetDirty();

				if(lootable->m_playerTag.IsSet())
				{
					lootable->m_extraLootTableId = auras->GetLootTableId(*aContext->m_random, GetManifest());

					if(npc->m_heroic && lootable->m_heroicLootTableId != 0)
						lootable->m_lootTableId = lootable->m_heroicLootTableId;

					const MapData* mapData = aContext->m_worldView->WorldViewGetMapData();
					if(lootable->m_lootTableId != 0 || mapData->m_mapInfo.m_mapLootTableIds.size() > 0 || lootable->m_extraLootTableId != 0)
						aContext->m_eventQueue->EventQueueGenerateLoot(aEntityInstanceId);
				}
			}

			npc->m_castInProgress.reset();

			const Components::NPC::StateEntry* deadState = npc->GetState(EntityState::ID_DEAD);
			if (deadState != NULL && deadState->m_barks.size() > 0)
			{
				const Chat* bark = Helpers::RandomItemPointer(*aContext->m_random, deadState->m_barks);
				assert(bark != NULL);
				aContext->m_eventQueue->EventQueueChat(aEntityInstanceId, *bark);
			}

			if(!npc->m_displayNameWhenDead.empty())
			{
				// FIXME: Here we really need optional components as well. To be honest, just get rid of the whole "required components" concept
				EntityInstance* entityInstance = (EntityInstance*)aContext->m_worldView->WorldViewSingleEntityInstance(aEntityInstanceId);
				Components::DisplayName* displayName = entityInstance != NULL ? entityInstance->GetComponent<Components::DisplayName>() : NULL;
				if (displayName != NULL)
				{
					displayName->m_string = npc->m_displayNameWhenDead;
					displayName->SetDirty();
				}
			}

			return EntityState::ID_DEAD;
		}

		if(aEntityState == EntityState::ID_DEAD && !threat->m_table.IsEmpty())
			aContext->m_eventQueue->EventQueueThreatClear(aEntityInstanceId);

		npc->m_cooldowns.Update(aContext->m_tick);

		Vec2 leashPosition = aEntityState == EntityState::ID_IN_COMBAT ? npc->m_lastTargetPosition : position->m_position;
		int32_t leashDistanceSquared = aEntityState == EntityState::ID_IN_COMBAT ? npc->m_anchorPosition.DistanceSquared(leashPosition) : 0;
		int32_t minLeashRangeSquared = GetManifest()->m_npcMetrics->m_minLeashRange * GetManifest()->m_npcMetrics->m_minLeashRange;

		bool isSurvivalMap = aContext->m_worldView->WorldViewGetMapData()->m_mapInfo.m_survivalScriptId != 0;

		if(!isSurvivalMap && npc->m_encounterId == 0 && (leashDistanceSquared >= minLeashRangeSquared || !npc->m_canMove))
		{
			std::vector<SourceEntityInstance> threatRemovedEntities;
			threat->m_table.Update(aContext->m_tick, threatRemovedEntities);

			for (const SourceEntityInstance& threatRemovedEntity : threatRemovedEntities)
				aContext->m_eventQueue->EventQueueThreat(threatRemovedEntity, aEntityInstanceId, INT32_MIN, aContext->m_tick, 0);
		}
		else if(npc->m_inactiveEncounterDespawn) 
		{
			bool validDespawnState = npc->m_inactiveEncounterDespawnState != EntityState::INVALID_ID && npc->m_inactiveEncounterDespawnState == aEntityState;
			if(validDespawnState && !aContext->m_worldView->WorldViewIsEncounterActive(npc->m_encounterId))
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
					{ aEntityInstanceId, 0 },
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
				topThreatEntity = aContext->m_worldView->WorldViewSingleEntityInstance(topThreatEntry->m_key.m_entityInstanceId);
				if(topThreatEntity != NULL && (topThreatEntity->GetState() == EntityState::ID_DEAD || topThreatEntry->m_key.m_entityInstanceSeq != topThreatEntity->GetSeq()))
					topThreatEntity = NULL;
			}

			bool passive = (aEntityState == EntityState::ID_DEFAULT) && aTicksInState < 15;

			if(aContext->m_tick - threat->m_lastPingTick >= Components::ThreatTarget::PING_INTERVAL_TICKS && !passive)
			{				
				bool blind = auras->HasEffect(AuraEffect::ID_BLIND, NULL);

				IWorldView::EntityQuery entityQuery;
				entityQuery.m_position = position->m_position;
				entityQuery.m_maxDistance = GetManifest()->m_npcMetrics->GetMaxAggroRange();
				entityQuery.m_flags = IWorldView::EntityQuery::FLAG_LINE_OF_SIGHT;

				aContext->m_worldView->WorldViewQueryEntityInstances(entityQuery, [&](
					const EntityInstance* aEntity,
					int32_t				  aDistanceSquared)
				{
					if(aEntity->GetState() != EntityState::ID_DEAD)
					{
						if (aEntityState == EntityState::ID_DEFAULT && aEntity->HasComponent<Components::ThreatSource>())
						{							
							const Components::CombatPublic* targetCombatPublic = aEntity->GetComponent<tpublic::Components::CombatPublic>();
							bool canAggro = !faction->IsNeutralOrFriendly() ||
								(faction->IsPVP() && faction->m_id != targetCombatPublic->m_factionId);

							if(canAggro || combat->m_targetEntityInstanceId == aEntity->GetEntityInstanceId())
							{
								int32_t aggroRange = 0;

								if (blind)
								{
									aggroRange = 1;
								}
								else
								{
									aggroRange = GetManifest()->m_npcMetrics->GetAggroRangeForLevelDifference((int32_t)combat->m_level, (int32_t)targetCombatPublic->m_level);

									aggroRange += npc->m_aggroRangeBias;

									if(aggroRange < 1)
										aggroRange = 1;
								}

								if (targetCombatPublic->m_stealthLevel > 0)
								{
									if (aggroRange > 1)
										aggroRange--;
								}

								if (aDistanceSquared <= aggroRange * aggroRange)
								{
									bool detected = true;

									if (targetCombatPublic->m_stealthLevel > 0)
									{
										uint32_t detectionChance = StealthUtils::GetOneSecondDetectionChance(aDistanceSquared, targetCombatPublic->m_stealthLevel, combat->m_level, targetCombatPublic->m_level);
										uint32_t roll = (*aContext->m_random)();
										detected = roll < detectionChance;
									}

									if (detected && (npc->m_aggroRequirements.m_requirements.empty() || Requirements::CheckList(GetManifest(), npc->m_aggroRequirements.m_requirements, NULL, aEntity)))
										aContext->m_eventQueue->EventQueueThreat({ aEntity->GetEntityInstanceId(), aEntity->GetSeq() }, aEntityInstanceId, 1, aContext->m_tick, 0);
								}
							}
						}
						else if (topThreatEntity != NULL && aEntity->GetState() == EntityState::ID_DEFAULT && (!faction->IsNeutralOrFriendly() || faction->IsPVP()))
						{
							const Components::CombatPublic* nearbyNonPlayerCombatPublic = aEntity->GetComponent<Components::CombatPublic>();
							int32_t aggroAssistRange = GetManifest()->m_npcMetrics->m_aggroAssistRange;
							if(nearbyNonPlayerCombatPublic != NULL && nearbyNonPlayerCombatPublic->m_factionId == combat->m_factionId && aDistanceSquared <= aggroAssistRange * aggroAssistRange)
							{	
								int32_t threatTick = aContext->m_tick; // threat->m_table.GetTick(); (FIXME: which makes most sense)
								aContext->m_eventQueue->EventQueueThreat({ topThreatEntity->GetEntityInstanceId(), topThreatEntity->GetSeq() }, aEntity->GetEntityInstanceId(), 1, threatTick, 0);
							}
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
				if(npc->m_despawnTime.m_immediatelyWhenDead)
				{
					returnValue = EntityState::ID_DESPAWNED;
				}
				else if(!npc->m_despawnTime.m_never)
				{
					const Components::Lootable* lootable = GetComponent<Components::Lootable>(aComponents);
					bool hasLoot = lootable->HasLoot();
					if((hasLoot && aTicksInState >= npc->m_despawnTime.m_ticksWithLoot) || 
						(!hasLoot && aTicksInState >= npc->m_despawnTime.m_ticksWithoutLoot))
					{
						returnValue = EntityState::ID_DESPAWNING;
					}
				}

				npc->m_targetEntity = SourceEntityInstance();
			}
			break;

		case EntityState::ID_DEFAULT:
			if(npc->m_spawnWithTarget.has_value())
			{
				aContext->m_eventQueue->EventQueueThreat(npc->m_spawnWithTarget->m_entity, aEntityInstanceId, npc->m_spawnWithTarget->m_threat, aContext->m_tick, 0);
				npc->m_spawnWithTarget.reset();
			}
			else if (!threat->m_table.IsEmpty())
			{
				threat->m_lastPingTick = 0; // Force a threat ping (will cause nearby allies to join)

				npc->m_anchorPosition = position->m_position; // Remember this position, this is where we'll go back to if we evade

				npc->m_targetEntity = threat->m_table.GetTop()->m_key;

				npc->m_patrolResetAfterLeavingCombat = npc->m_npcBehaviorState != NULL && npc->m_npcBehaviorState->m_patrolResetAfterLeavingCombat;
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

				if(npc->m_npcBehaviorState->m_onRoute)
				{
					const MapRouteData* mapRouteData = aContext->m_worldView->WorldViewGetMapData()->m_mapRouteData.get();
					if (mapRouteData != NULL)
					{
						npc->m_subRouteIndex = mapRouteData->GetSubRouteIndexByPosition(npc->m_npcBehaviorState->m_onRoute->m_routeId, position->m_position);
						if(npc->m_subRouteIndex != SIZE_MAX)
						{
							npc->m_effectiveRouteId = npc->m_npcBehaviorState->m_onRoute->m_routeId;
							npc->m_npcBehaviorState = GetManifest()->GetById<Data::NPCBehaviorState>(npc->m_npcBehaviorState->m_onRoute->m_npcBehaviorStateId);
						}
					}					
				}
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
					case NPCBehavior::ID_SURVIVAL_WAVE:
					case NPCBehavior::ID_SURVIVAL_BOSS:
						if(npc->m_npcBehaviorState->m_despawnIfLostPlayer)
						{
							if(!aContext->m_worldView->WorldViewIsSurvivalActive())
								returnValue = EntityState::ID_DESPAWNING;
						}
						break;

					case NPCBehavior::ID_PATROLLING:
						{
							bool paused = false;

							if (npc->m_npcBehaviorState->m_despawnIfLostPlayer && npc->m_effectiveRouteId != 0)
							{
								const Components::Tag* tag = GetComponent<Components::Tag>(aComponents);

								bool despawn = false;

								if (tag->m_playerEntityInstanceId == 0)
									despawn = true;

								if (!despawn)
								{
									const EntityInstance* taggedByEntityInstance = aContext->m_worldView->WorldViewSingleEntityInstance(tag->m_playerEntityInstanceId);
									const Components::Position* taggedByEntityPosition = taggedByEntityInstance != NULL ? taggedByEntityInstance->GetComponent<Components::Position>() : NULL;
									despawn = taggedByEntityPosition == NULL || taggedByEntityInstance->GetState() == EntityState::ID_DEAD || !Helpers::IsWithinDistance(position, taggedByEntityPosition, 48);
									paused = taggedByEntityInstance != NULL && taggedByEntityInstance->GetState() == EntityState::ID_IN_COMBAT;
								}

								if (despawn)
									returnValue = EntityState::ID_DESPAWNING;
							}

							if (!paused && npc->m_moveCooldownUntilTick < aContext->m_tick && npc->m_effectiveRouteId != 0)
							{								
								if (npc->m_npcBehaviorState->m_combatEventPauseTicks != 0)
									paused = aContext->m_tick - combat->m_lastCombatEventTick <= npc->m_npcBehaviorState->m_combatEventPauseTicks;

								if (!paused && npc->m_npcBehaviorState->m_pauseWhenTargetedByNearbyPlayer)
								{
									std::vector<uint32_t> entityIds = { 0 }; // Players

									IWorldView::EntityQuery entityQuery;
									entityQuery.m_position = position->m_position;
									entityQuery.m_maxDistance = 3;
									entityQuery.m_entityIds = &entityIds;
									aContext->m_worldView->WorldViewQueryEntityInstances(entityQuery, [&](
										const EntityInstance* aEntityInstance,
										int32_t /*aDistanceSquared*/) -> bool
									{
										if (aEntityInstance->GetComponent<Components::CombatPublic>()->m_targetEntityInstanceId == aEntityInstanceId)
										{
											paused = true;
											return true;
										}
										return false;
									});
								}

								if (!paused)
								{
									const MapRouteData* mapRouteData = aContext->m_worldView->WorldViewGetMapData()->m_mapRouteData.get();
									if (mapRouteData != NULL)
									{
										if (npc->m_subRouteIndex == SIZE_MAX)
											npc->m_subRouteIndex = mapRouteData->GetSubRouteIndexByPosition(npc->m_effectiveRouteId, position->m_position);											

										Vec2 direction;
										bool shouldChangeDirection = false;
										std::optional<uint32_t> index;

										if (mapRouteData->GetDirection(npc->m_effectiveRouteId, npc->m_subRouteIndex, position->m_position, npc->m_routeIsReversing, direction, shouldChangeDirection, index))
										{
											IEventQueue::EventQueueMoveRequest moveRequest;
											moveRequest.AddToPriorityList(direction);
											moveRequest.m_type = IEventQueue::EventQueueMoveRequest::TYPE_SIMPLE;
											moveRequest.m_entityInstanceId = aEntityInstanceId;
											moveRequest.m_canMoveOnAllNonViewBlockingTiles = npc->m_canMoveOnAllNonViewBlockingTiles;

											aContext->m_eventQueue->EventQueueMove(moveRequest);

											if (shouldChangeDirection)
												npc->m_routeIsReversing = !npc->m_routeIsReversing;

											if (index.has_value())
											{
												const Data::Route* routeData = GetManifest()->GetById<Data::Route>(npc->m_effectiveRouteId);
												for (size_t triggerIndex = 0; triggerIndex < routeData->m_triggers.size(); triggerIndex++)
												{
													const std::unique_ptr<Data::Route::Trigger>& trigger = routeData->m_triggers[triggerIndex];
													if (trigger->m_index == index.value() && !npc->m_handledRouteTriggerIndices.contains(trigger->m_index))
													{
														if (trigger->m_chat)
															aContext->m_eventQueue->EventQueueChat(aEntityInstanceId, trigger->m_chat.value());

														if (trigger->m_event)
														{
															const Components::Tag* tag = GetComponent<Components::Tag>(aComponents);
															if (tag->m_playerTag.IsSet())
																aContext->m_eventQueue->EventQueueEntityObjective(tag->m_playerTag, aEntityId, EntityObjectiveEvent::TYPE_ROUTE_NPC, 24, position->m_position, tag->m_playerEntityInstanceId);
														}

														if (trigger->m_abilityId != 0)
														{
															const Data::Ability* ability = GetManifest()->GetById<Data::Ability>(trigger->m_abilityId);

															aContext->m_eventQueue->EventQueueAbility(
																SourceEntityInstance{ aEntityInstanceId, 0 },
																aEntityInstanceId,
																Vec2(),
																ability);
														}

														npc->m_handledRouteTriggerIndices.insert(trigger->m_index);

														if (trigger->m_despawn)
														{
															returnValue = EntityState::ID_DESPAWNING;
														}
														else if (trigger->m_reset)
														{
															npc->m_anchorPosition = npc->m_spawnPosition;
															npc->m_npcBehaviorState = NULL;
															npc->m_subRouteIndex = SIZE_MAX;
															npc->m_effectiveRouteId = 0;
															npc->m_handledRouteTriggerIndices.clear();

															returnValue = EntityState::ID_EVADING;
														}
													}
												}
											}
										}
									}
								}

								if (npc->m_npcBehaviorState != NULL)
									npc->m_moveCooldownUntilTick = aContext->m_tick + npc->m_npcBehaviorState->m_patrolMoveIntervalTicks + ((*aContext->m_random)() & 1);
							}
						}
						break;

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
								moveRequest.m_canMoveOnAllNonViewBlockingTiles = npc->m_canMoveOnAllNonViewBlockingTiles;

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

								if(ability->m_mustNotHaveWorldAuraId != 0 && aContext->m_worldView->WorldViewHasWorldAura(ability->m_mustNotHaveWorldAuraId))
									continue;				
								
								if (abilityEntry.m_useProbability == UINT32_MAX || (*aContext->m_random)() < abilityEntry.m_useProbability)
								{
									uint32_t targetEntityInstanceId = 0;

									if(abilityEntry.m_targetEntityIds.size() > 0)
									{
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
									}
									else if(abilityEntry.m_targetType == Components::NPC::AbilityEntry::TARGET_TYPE_RANDOM_PLAYER)										
									{
										const EntityInstance* self = aContext->m_worldView->WorldViewSingleEntityInstance(aEntityInstanceId);

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
											{
												if(Requirements::CheckList(GetManifest(), abilityEntry.m_requirements, self, aEntity))
													possibleTargets.push_back(aEntity);
											}
											return false;
										});

										if(possibleTargets.size() > 0)
										{									
											const EntityInstance* selectedTarget = possibleTargets[Helpers::RandomInRange<size_t>(*aContext->m_random, 0, possibleTargets.size() - 1)];
											
											targetEntityInstanceId = selectedTarget->GetEntityInstanceId();
										}
									}
									else if(abilityEntry.m_targetType == Components::NPC::AbilityEntry::TARGET_TYPE_SELF)
									{
										targetEntityInstanceId = aEntityInstanceId;
									}

									if (targetEntityInstanceId != 0)
									{
										if (abilityEntry.m_requirements.size() > 0)
										{
											const EntityInstance* self = aContext->m_worldView->WorldViewSingleEntityInstance(aEntityInstanceId);
											const EntityInstance* target = aContext->m_worldView->WorldViewSingleEntityInstance(targetEntityInstanceId);
											if (self != NULL && target != NULL && !Requirements::CheckList(GetManifest(), abilityEntry.m_requirements, self, target))
												continue;
										}

										if (abilityEntry.m_barks.size() > 0)
										{
											const Chat* bark = Helpers::RandomItemPointer(*aContext->m_random, abilityEntry.m_barks);
											assert(bark != NULL);
											aContext->m_eventQueue->EventQueueChat(aEntityInstanceId, *bark);
										}

										npc->m_cooldowns.AddAbility(GetManifest(), ability, aContext->m_tick, 0.0f, NULL); // FIXME: cooldown modifier from haste?

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
											aContext->m_eventQueue->EventQueueAbility({ aEntityInstanceId, 0 }, targetEntityInstanceId, Vec2(), ability, ItemInstanceReference(), NULL);
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
					npc->m_targetEntity = SourceEntityInstance();
					npc->m_castInProgress.reset();
					npc->m_npcMovement.Reset(aContext->m_tick);				

					Components::Tag* tag = GetComponent<Components::Tag>(aComponents);
					tag->m_playerTag.Clear();
					tag->SetDirty();

					position->m_lastMoveTick = aContext->m_tick;

					if(npc->m_patrolResetAfterLeavingCombat)
					{
						npc->m_anchorPosition = npc->m_spawnPosition;
						npc->m_npcBehaviorState = NULL;
						npc->m_subRouteIndex = SIZE_MAX;
						npc->m_effectiveRouteId = 0;
						npc->m_handledRouteTriggerIndices.clear();
					}

					// Reset kill contribution (if any). FIXME: this is ugly - entities should support optional components
					{
						EntityInstance* entityInstance = (EntityInstance*)aContext->m_worldView->WorldViewSingleEntityInstance(aEntityInstanceId);
						Components::KillContribution* killContribution = entityInstance != NULL ? entityInstance->GetComponent<Components::KillContribution>() : NULL;
						if(killContribution != NULL)
							killContribution->Reset();
					}

					returnValue = EntityState::ID_EVADING;
				}
				else 
				{	
					if(npc->m_castInProgress)
						npc->m_lastCombatMoveTick = aContext->m_tick; // Casting counts as moving in combat, otherwise casters would reset their leash all the time

					bool standingStill = (aContext->m_tick - npc->m_lastCombatMoveTick) >= 10;
					bool isStunned = auras->HasEffect(AuraEffect::ID_STUN, NULL);

					if ((standingStill && npc->m_canMove) || isStunned)
						threat->m_table.Touch(aContext->m_tick);

					if (!isStunned)
					{
						if(!auras->HasEffect(AuraEffect::ID_TAUNT, &npc->m_targetEntity))
						{
							const ThreatTable::Entry* topThreatEntry = threat->m_table.GetTop();

							if(topThreatEntry->m_key != npc->m_targetEntity)
							{
								int32_t existingTargetThreat = npc->m_targetEntity.IsSet() ? threat->m_table.GetThreat(npc->m_targetEntity) : 0;

								if(existingTargetThreat != 0)
								{
									// To switch the new target must have 8% more threat if in melee range or 25% more threat if outside
									bool newTargetInMeleeRange = _IsInMeleeRange(position, aContext->m_worldView->WorldViewSingleEntityInstance(topThreatEntry->m_key.m_entityInstanceId));
									int32_t newTargetThreatPercentageIncrease = (100 * (topThreatEntry->m_threat - existingTargetThreat)) / existingTargetThreat;

									if((newTargetInMeleeRange && newTargetThreatPercentageIncrease >= 8) || (!newTargetInMeleeRange && newTargetThreatPercentageIncrease >= 25))
										npc->m_targetEntity = topThreatEntry->m_key;
								}
								else
								{
									npc->m_targetEntity = topThreatEntry->m_key;
								}								
							}
						}					
						
						const EntityInstance* target = aContext->m_worldView->WorldViewSingleEntityInstance(npc->m_targetEntity.m_entityInstanceId);

						if(target != NULL)
						{
							if (npc->m_zoneId != 0 && (npc->m_outOfZoneAction.m_useAbilityId != 0 || npc->m_outOfZoneAction.m_evade))
							{
								const Components::Position* targetPosition = target->GetComponent<Components::Position>();
								const WorldInfoMap::Entry& worldInfoMapEntry = aContext->m_worldView->WorldViewGetMapData()->m_worldInfoMap->Get(targetPosition->m_position);
								bool inZone = worldInfoMapEntry.m_zoneId == npc->m_zoneId || worldInfoMapEntry.m_subZoneId == npc->m_zoneId;

								if(!inZone)
								{
									if(npc->m_outOfZoneAction.m_evade)
									{
										target = NULL; // Pretend target doesn't exist anymore
									}
									else if(npc->m_outOfZoneAction.m_useAbilityId != 0)
									{
										const Data::Ability* ability = GetManifest()->GetById<tpublic::Data::Ability>(npc->m_outOfZoneAction.m_useAbilityId);
										// FIXME: just ignoring everything except the cooldown and always target self
										if (!npc->m_cooldowns.IsAbilityOnCooldown(ability))
										{
											npc->m_cooldowns.AddAbility(GetManifest(), ability, aContext->m_tick, 0.0f, NULL);
											aContext->m_eventQueue->EventQueueAbility({ aEntityInstanceId, 0 }, aEntityInstanceId, Vec2(), ability, ItemInstanceReference(), NULL);
										}
									}
								}
							}
						}

						if (target == NULL || target->GetState() == EntityState::ID_DEAD || target->GetSeq() != npc->m_targetEntity.m_entityInstanceSeq)
						{
							aContext->m_eventQueue->EventQueueThreat(npc->m_targetEntity, aEntityInstanceId, INT32_MIN, aContext->m_tick, 0);

							npc->m_castInProgress.reset();
							npc->m_targetEntity = SourceEntityInstance();
						}
						else if (state != NULL && !npc->m_castInProgress)						
						{
							const Components::Position* targetPosition = target->GetComponent<Components::Position>();
							npc->m_lastTargetPosition = targetPosition->m_position;
							int32_t distanceSquared = Helpers::CalculateDistanceSquared(targetPosition, position);

							const Data::Ability* useAbility = NULL;
							SourceEntityInstance useAbilityTarget = npc->m_targetEntity;
							const Components::NPC::AbilityEntry* useAbilityEntry = NULL;

							for (const Components::NPC::AbilityEntry& abilityEntry : state->m_abilities)
							{
								const Data::Ability* ability = GetManifest()->GetById<tpublic::Data::Ability>(abilityEntry.m_abilityId);

								if (ability->m_mustNotHaveWorldAuraId != 0 && aContext->m_worldView->WorldViewHasWorldAura(ability->m_mustNotHaveWorldAuraId))
									continue;

								if(abilityEntry.m_requirements.size() > 0)
								{
									const EntityInstance* self = aContext->m_worldView->WorldViewSingleEntityInstance(aEntityInstanceId);
									if(self != NULL && !Requirements::CheckList(GetManifest(), abilityEntry.m_requirements, self, target))
										continue;
								}

								if (npc->m_encounterId != 0 && aContext->m_worldView->WorldViewIsEncounterBlockingNPCAbility(npc->m_encounterId, ability->m_id))
									continue;

								if (abilityEntry.m_targetType == Components::NPC::AbilityEntry::TARGET_TYPE_DEFAULT)
								{
									if (!ability->AlwaysInRange() && distanceSquared > (int32_t)(ability->m_range * ability->m_range))
										continue;

									if (distanceSquared < (int32_t)(ability->m_minRange * ability->m_minRange))
										continue;

									if (npc->m_cooldowns.IsAbilityOnCooldown(ability))
										continue;

									if (!combat->HasResourcesForAbility(ability, NULL, combat->GetResourceMax(Resource::ID_MANA)))
										continue;

									if(!ability->AlwaysInLineOfSight() && !aContext->m_worldView->WorldViewLineOfSight(targetPosition->m_position, position->m_position))
										continue;

									if (abilityEntry.m_useProbability == UINT32_MAX || (*aContext->m_random)() < abilityEntry.m_useProbability)
									{
										useAbility = ability;
										useAbilityEntry = &abilityEntry;
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
										useAbilityEntry = &abilityEntry;
										useAbilityTarget = { selectedTarget->GetEntityInstanceId(), selectedTarget->GetSeq() };
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
										useAbilityEntry = &abilityEntry;
										useAbilityTarget = { selectedTarget->GetEntityInstanceId(), selectedTarget->GetSeq() };
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
									useAbilityEntry = &abilityEntry;
									useAbilityTarget = { aEntityInstanceId, 0 }; // We're assuming that NPC seq is always zero
									break;
								}
							}

							if (useAbility != NULL)
							{
								if(useAbilityEntry != NULL && useAbilityEntry->m_updateTarget && useAbilityTarget.IsSet())
									npc->m_targetEntity = useAbilityTarget;

								position->m_lastMoveTick = aContext->m_tick;
								npc->m_npcMovement.Reset(aContext->m_tick);

								if(useAbility->IsOffensive())
								{
									npc->m_lastAttackTick = aContext->m_tick;
								}

								float cooldownModifier = 0.0f;
								if(useAbility->IsAttack() && useAbility->IsMelee())
									cooldownModifier = auras->GetAttackHaste(GetManifest());

								npc->m_cooldowns.AddAbility(GetManifest(), useAbility, aContext->m_tick, cooldownModifier, NULL);

								if (useAbilityEntry != NULL && useAbilityEntry->m_barks.size() > 0)
								{
									const Chat* bark = Helpers::RandomItemPointer(*aContext->m_random, useAbilityEntry->m_barks);
									assert(bark != NULL);
									aContext->m_eventQueue->EventQueueChat(aEntityInstanceId, *bark);
								}

								if(useAbility->m_castTime > 0)
								{	
									int32_t castTime = useAbility->m_castTime;
									if(useAbility->IsSpell())
										castTime = Haste::CalculateCastTime(castTime, auras->GetSpellHaste(GetManifest()));

									CastInProgress cast;
									cast.m_abilityId = useAbility->m_id;
									cast.m_targetEntityInstanceId = useAbilityTarget.m_entityInstanceId;
									cast.m_start = aContext->m_tick;
									cast.m_end = cast.m_start + castTime;
									npc->m_castInProgress = cast;
								}
								else
								{
									aContext->m_eventQueue->EventQueueAbility({ aEntityInstanceId, 0 }, useAbilityTarget.m_entityInstanceId, Vec2(), useAbility, ItemInstanceReference(), NULL);
								}
							}
							else if(distanceSquared > 1 && !auras->HasEffect(AuraEffect::ID_IMMOBILIZE, NULL))
							{
								int32_t ticksSinceLastAttack = aContext->m_tick - npc->m_lastAttackTick;
								if(ticksSinceLastAttack > 30 && npc->m_maxLeashDistance > 0 && leashDistanceSquared > (int32_t)(npc->m_maxLeashDistance * npc->m_maxLeashDistance))
								{
									// FIXME: don't do this in dungeons

									// Exceeded max leash distance nad haven't been able to attack for a bit. Evade!
									aContext->m_eventQueue->EventQueueThreatClear(aEntityInstanceId);
								}
								else if(ticksSinceLastAttack > 80 && npc->m_large)
								{
									// We're a large NPC and we haven't been able to hurt anyone for some time. We're probably getting cheesed. Evade!
									aContext->m_eventQueue->EventQueueThreatClear(aEntityInstanceId);
								}
								else if(npc->m_canMove)
								{
									const MoveSpeed::Info* moveSpeedInfo = MoveSpeed::GetInfo(combat->m_moveSpeed);
									if (npc->m_moveCooldownUntilTick + moveSpeedInfo->m_tickBias < aContext->m_tick)
									{
										if (npc->m_npcMovement.ShouldResetIfLOS(aContext->m_tick))
										{
											if (aContext->m_worldView->WorldViewLineOfSight(position->m_position, targetPosition->m_position))
												npc->m_npcMovement.Reset(aContext->m_tick);
										}

										const EntityInstance* pushEntityInstance = NULL;

										if(npc->m_meleePushPriority > 0 && !npc->m_large)
										{
											std::function<const EntityInstance*(const Vec2&)> getEntityInstance = [&](
												const Vec2& aPosition) -> const EntityInstance*
											{
												const EntityInstance* entityInstance = NULL;

												aContext->m_worldView->WorldViewEntityInstancesAtPosition(aPosition, [&](
													const EntityInstance* aMidEntityInstance) -> bool
												{
													const Components::NPC* otherNPC = aMidEntityInstance->GetComponent<Components::NPC>();
													const Components::CombatPublic* otherCombatPublic = aMidEntityInstance->GetComponent<Components::CombatPublic>();
													const Components::Position* otherPosition = aMidEntityInstance->GetComponent<Components::Position>();

													if(otherNPC != NULL && otherCombatPublic != NULL)
													{
														bool canBePushed = otherCombatPublic->IsPushable() || otherNPC->m_otherNPCPushOverride;
														if (otherNPC->m_meleePushPriority < npc->m_meleePushPriority && canBePushed && !otherNPC->m_large && otherPosition->IsBlocking())
														{
															entityInstance = aMidEntityInstance;
															return true; // Stop, found something to push
														}
													}										
													return false;
												});

												return entityInstance;
											};

											if(distanceSquared == 4)
											{
												// This means that we're horizontally or vertically 2 tiles away from the target - and we're some kind of melee NPC that really wants 
												// to get close to attack. Figure out if some other NPC is standing between us and the target.
												// FIXME: doesn't work if either of the NPCs are large
												Vec2 midPosition = { (position->m_position.m_x + targetPosition->m_position.m_x) / 2, (position->m_position.m_y + targetPosition->m_position.m_y) / 2 };
												pushEntityInstance = getEntityInstance(midPosition);
											}
											else if(distanceSquared == 2)
											{
												// Same as above, but diagonally adjacent. This is a bit more complicated, because we have two spots to check.
												Vec2 midPositions[2] = 
												{
													{ position->m_position.m_x, targetPosition->m_position.m_y },
													{ targetPosition->m_position.m_x, position->m_position.m_y }
												};

												const EntityInstance* midEntityInstances[2] = { NULL, NULL };
												for(size_t i = 0; i < 2; i++)
													midEntityInstances[i] = getEntityInstance(midPositions[i]);

												if(midEntityInstances[0] != NULL && midEntityInstances[1] == NULL)
													pushEntityInstance = midEntityInstances[0];
												else if (midEntityInstances[1] != NULL && midEntityInstances[0] == NULL)
													pushEntityInstance = midEntityInstances[1];
											}
										}

										if(pushEntityInstance != NULL)
										{
											const Data::Ability* pushAbility = GetManifest()->GetExistingByName<Data::Ability>("npc_push");

											aContext->m_eventQueue->EventQueueAbility({ aEntityInstanceId, 0 }, pushEntityInstance->GetEntityInstanceId(), Vec2(), pushAbility, ItemInstanceReference(), NULL);
										}
										else
										{	
											if(position->m_blockingEntityInstanceId != 0)
											{
												const EntityInstance* blockingEntityInstance = aContext->m_worldView->WorldViewSingleEntityInstance(position->m_blockingEntityInstanceId);
												if(blockingEntityInstance != NULL && blockingEntityInstance->HasComponent<Components::ThreatSource>())
												{
													// FIXME: this could be abused by tricking unsuspecting players to block a mob. But if we put on some kind of "must be in same group" requirement,
													// it could be abused the other way around.

													// Basically let the blocking player/minion taunt - AND - reduce threat on current target to get a fast switch
													aContext->m_eventQueue->EventQueueThreat(	
														{ blockingEntityInstance->GetEntityInstanceId(), blockingEntityInstance->GetSeq() },
														aEntityInstanceId,
														INT32_MAX,
														aContext->m_tick,
														0);

													aContext->m_eventQueue->EventQueueThreat(npc->m_targetEntity, aEntityInstanceId, -1, 0, 0, 0.8f);
												}
												else
												{
													position->m_blockingEntityInstanceId = 0;
												}
											}

											IEventQueue::EventQueueMoveRequest moveRequest;
											if (npc->m_npcMovement.GetMoveRequest(
												aContext->m_worldView->WorldViewGetMapData()->m_mapPathData.get(),
												position->m_position,
												targetPosition->m_position,
												aContext->m_tick,
												position->m_lastMoveTick,
												npc->m_lastAttackTick,
												*aContext->m_random,
												moveRequest))
											{
												moveRequest.m_entityInstanceId = aEntityInstanceId;
												moveRequest.m_canMoveOnAllNonViewBlockingTiles = npc->m_canMoveOnAllNonViewBlockingTiles;

												aContext->m_eventQueue->EventQueueMove(moveRequest);

												npc->m_moveCooldownUntilTick = aContext->m_tick + npc->m_combatMoveIntervalTicks;
												npc->m_lastCombatMoveTick = aContext->m_tick;
												npc->m_moveFailureAccum = 0;
											}
											else 
											{
												npc->m_moveFailureAccum++;

												if(npc->m_moveFailureAccum > 1)
												{
													// Seems like we're stuck chasing the top threat target. Reduce threat on that one.
													position->m_lastMoveTick = aContext->m_tick;

													aContext->m_eventQueue->EventQueueThreat(npc->m_targetEntity, aEntityInstanceId, -1, 0, 0, 0.5f);
												}
											}
										}
									}
								}
								else
								{
									position->m_lastMoveTick = aContext->m_tick;

									aContext->m_eventQueue->EventQueueThreat(npc->m_targetEntity, aEntityInstanceId, -1, 0, 0, 0.5f);
								}
							}
						}
					}
				}
			}
			break;

		case EntityState::ID_EVADING:
			if(npc->m_evadeDespawn)
			{
				returnValue = EntityState::ID_DESPAWNING;;
			}
			else if(position->m_position == npc->m_anchorPosition)
			{
				npc->m_restoreResources = true;

				returnValue = EntityState::ID_DEFAULT;
			}
			else if(npc->m_moveCooldownUntilTick < aContext->m_tick)
			{
				// FIXME: since (future me: what?)
				IEventQueue::EventQueueMoveRequest moveRequest;
				if (!npc->m_npcMovement.GetMoveRequest(aContext->m_worldView->WorldViewGetMapData()->m_mapPathData.get(), position->m_position, npc->m_anchorPosition, aContext->m_tick, position->m_lastMoveTick, 0, *aContext->m_random, moveRequest))
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

		if(despawnOnLeaveState && returnValue != EntityState::CONTINUE && returnValue != EntityState::DESTROY)
			return EntityState::ID_DESPAWNING;

		return returnValue;
	}

	void			
	NPC::UpdatePublic(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			aContext) 
	{
		Components::CombatPublic* combat = GetComponent<Components::CombatPublic>(aComponents);
		Components::NPC* npc = GetComponent<Components::NPC>(aComponents);
		Components::Position* position = GetComponent<Components::Position>(aComponents);

		if (combat->IsElite())
		{
			bool isEasyElitesEnabled = aContext->m_worldView->WorldViewGetRealmModifierList()->GetFlag(RealmModifier::ID_EASY_ELITES, false);

			if (isEasyElitesEnabled)
			{
				// FIXME: if a component is static (like CombatPrivate often is), this will get messed up if entity updates ever get parallelized
				Components::CombatPrivate* combatPrivate = GetComponent<Components::CombatPrivate>(aComponents);

				ApplyNPCMetrics::MakeEliteEasy(GetManifest()->m_npcMetrics.get(), combat, combatPrivate);
			}
		}

		if(npc->m_restoreResources)
		{
			combat->SetDefaultResources();
			combat->SetDirty();

			npc->m_restoreResources = false;
		}

		if(combat->m_targetEntityInstanceId != npc->m_targetEntity.m_entityInstanceId)
		{
			combat->m_targetEntityInstanceId = npc->m_targetEntity.m_entityInstanceId;
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
			 aEntityState != EntityState::ID_DESPAWNED) &&
			(npc->m_npcBehaviorState == NULL || !npc->m_npcBehaviorState->m_noBlocking);

		if(position->IsBlocking() != block)
		{
			if(block)
				position->SetBlocking();
			else
				position->ClearBlocking();
			position->SetDirty();
		}

		combat->m_interrupt.reset();

		{
			const Components::VisibleAuras* visibleAuras = GetComponent<Components::VisibleAuras>(aComponents);
			uint8_t stealthLevel = 0;
			if (visibleAuras->IsStealthed())
			{
				stealthLevel = 1;

				if (aContext->m_worldView->WorldViewGetMapData()->DoesNeighborTileBlockLineOfSight(position->m_position.m_x, position->m_position.m_y))
				{
					stealthLevel++; // More stealth from hugging a wall
					if (aContext->m_tick - position->m_lastMoveTick > 20)
						stealthLevel++; // More steal from standing still for at least 2 seconds while hugging a wall
				}
			}

			if (combat->m_stealthLevel != stealthLevel)
			{
				combat->m_stealthLevel = stealthLevel;
				combat->SetDirty();
			}
		}
	}

}

