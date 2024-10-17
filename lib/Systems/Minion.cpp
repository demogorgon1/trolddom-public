#include "../Pcheader.h"

#include <tpublic/Data/Faction.h>
#include <tpublic/Data/MinionMode.h>

#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/DisplayName.h>
#include <tpublic/Components/MinionPrivate.h>
#include <tpublic/Components/MinionPublic.h>
#include <tpublic/Components/NPC.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/ThreatSource.h>

#include <tpublic/Data/NameTemplate.h>

#include <tpublic/Systems/Minion.h>

#include <tpublic/CreateName.h>
#include <tpublic/EntityInstance.h>
#include <tpublic/Haste.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Manifest.h>
#include <tpublic/MapData.h>
#include <tpublic/WordList.h>

namespace tpublic::Systems
{

	namespace
	{
		bool
		_EvalutateFriendlyTarget(
			SystemBase::Context*						aContext,
			const Vec2&									aMinionPosition,
			const Vec2&									aPosition,
			const Components::CombatPublic*				aCombatPublic,
			const Data::Ability*						aAbility,
			const Components::MinionPublic::Ability*	aMinionAbilityInfo,
			int32_t										aMaxRange)
		{
			// At this point we assume the target is friendly
			int32_t distanceSquared = aPosition.DistanceSquared(aMinionPosition);
			int32_t maxRangeSquared = aMaxRange * aMaxRange;

			if(distanceSquared > maxRangeSquared)
				return false;
			if(distanceSquared < (int32_t)(aAbility->m_minRange * aAbility->m_minRange))
				return false;

			if(aMinionAbilityInfo->m_maxTargetHealth != 0)
			{
				uint32_t targetHealthPercentage = aCombatPublic->GetResourcePercentage(Resource::ID_HEALTH);

				if(targetHealthPercentage > aMinionAbilityInfo->m_maxTargetHealth)
					return false;
			}

			if(!aContext->m_worldView->WorldViewLineOfSight(aMinionPosition, aPosition))
				return false;

			return true;
		}

		uint32_t
		_PickFriendlyTarget(
			SystemBase::Context*						aContext,
			uint32_t									aEntityInstanceId,
			const Components::CombatPublic*				aCombatPublic,
			const Vec2&									aPosition,
			const EntityInstance*						aOwnerEntityInstance,
			const Data::Ability*						aAbility,
			const Components::MinionPublic::Ability*	aMinionAbilityInfo,
			int32_t										aMaxRange)
		{
			// Order of priority: self, owner, owner's combat group members

			uint32_t targetEntityInstanceId = 0;

			if(aAbility->TargetSelf() && _EvalutateFriendlyTarget(aContext, aPosition, aPosition, aCombatPublic, aAbility, aMinionAbilityInfo, aMaxRange))
				targetEntityInstanceId = aEntityInstanceId;

			const Components::CombatPublic* ownerCombatPublic = aOwnerEntityInstance->GetComponent<Components::CombatPublic>();

			if(targetEntityInstanceId == 0 && _EvalutateFriendlyTarget(
				aContext,
				aPosition, 
				aOwnerEntityInstance->GetComponent<Components::Position>()->m_position, 
				ownerCombatPublic,
				aAbility,
				aMinionAbilityInfo,
				aMaxRange))
			{
				targetEntityInstanceId = aOwnerEntityInstance->GetEntityInstanceId();
			}

			if(targetEntityInstanceId == 0 && ownerCombatPublic->m_combatGroupId != 0)
			{
				aContext->m_worldView->WorldViewGroupEntityInstances(ownerCombatPublic->m_combatGroupId, [&](
					const EntityInstance* aEntityInstance)
				{
					if(aEntityInstance != aOwnerEntityInstance)
					{
						if (_EvalutateFriendlyTarget(
							aContext,
							aPosition,
							aEntityInstance->GetComponent<Components::Position>()->m_position,
							aEntityInstance->GetComponent<Components::CombatPublic>(),
							aAbility,
							aMinionAbilityInfo,
							aMaxRange))
						{
							targetEntityInstanceId = aEntityInstance->GetEntityInstanceId();
							return true; // Got what we're looking ofr
						}
					}

					return false; // Continue
				});
			}

			return targetEntityInstanceId;
		}

	}

	//---------------------------------------------------------------------------

	Minion::Minion(
		const SystemData*	aData)
		: SystemBase(aData)
	{
		RequireComponent<Components::Auras>();
		RequireComponent<Components::CombatPublic>();
		RequireComponent<Components::DisplayName>();
		RequireComponent<Components::MinionPrivate>();
		RequireComponent<Components::MinionPublic>();
		RequireComponent<Components::Position>();
		RequireComponent<Components::ThreatSource>();
	}
	
	Minion::~Minion()
	{

	}

	//---------------------------------------------------------------------------

	void			
	Minion::Init(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		ComponentBase**		aComponents,
		int32_t				/*aTick*/) 
	{
		Components::MinionPrivate* minionPrivate = GetComponent<Components::MinionPrivate>(aComponents);
		Components::DisplayName* displayName = GetComponent<Components::DisplayName>(aComponents);

		// Name
		if(displayName->m_string.empty())
		{
			std::mt19937 random(minionPrivate->m_seed);
			TP_CHECK(minionPrivate->m_nameTemplateId != 0, "Missing minion name template.");			
			const Data::NameTemplate* nameTemplate = GetManifest()->GetById<Data::NameTemplate>(minionPrivate->m_nameTemplateId);
			WordList::QueryCache queryCache(&GetManifest()->m_wordList);
			std::unordered_set<uint32_t> tags;
			CreateName(GetManifest(), nameTemplate, &queryCache, random, displayName->m_string, tags);
			displayName->SetDirty();
		}

		// Initialize resources		
		{
			Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);

			for (const Components::MinionPrivate::ResourceEntry& resource : minionPrivate->m_resources.m_entries)
			{
				combatPublic->AddResourceMax(resource.m_id, resource.m_max);

				const Resource::Info* info = Resource::GetInfo((Resource::Id)resource.m_id);
				if (info->m_flags & Resource::FLAG_DEFAULT_TO_MAX)
					combatPublic->SetResourceToMax(resource.m_id);
			}

			combatPublic->SetDirty();
		}
	}
	
	EntityState::Id	
	Minion::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			aEntityInstanceId,
		EntityState::Id		aEntityState,
		int32_t				aTicksInState,
		ComponentBase**		aComponents,
		Context*			aContext) 
	{
		if (aEntityState == EntityState::ID_SPAWNING)
			return aTicksInState < SPAWN_TICKS ? EntityState::CONTINUE : EntityState::ID_DEFAULT;

		if (aEntityState == EntityState::ID_DESPAWNING)
			return aTicksInState < DESPAWN_TICKS ? EntityState::CONTINUE : EntityState::ID_DESPAWNED;

		Components::MinionPrivate* minionPrivate = GetComponent<Components::MinionPrivate>(aComponents);
		Components::MinionPublic* minionPublic = GetComponent<Components::MinionPublic>(aComponents);
		Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);
		Components::Position* position = GetComponent<Components::Position>(aComponents);
		const Components::Auras* auras = GetComponent<Components::Auras>(aComponents);
		Components::ThreatSource* threatSource = GetComponent<Components::ThreatSource>(aComponents);

		const EntityInstance* ownerEntityInstance = minionPublic->m_ownerEntityInstanceId != 0 ? aContext->m_worldView->WorldViewSingleEntityInstance(minionPublic->m_ownerEntityInstanceId) : NULL;
		if(ownerEntityInstance == NULL || ownerEntityInstance->GetState() == EntityState::ID_DEAD)
		{
			// No owner, gotta despawn.
			return EntityState::ID_DESPAWNING;
		}

		if (aEntityState != EntityState::ID_DEAD && combatPublic->GetResource(Resource::ID_HEALTH) == 0 && !auras->HasEffect(AuraEffect::ID_IMMORTALITY, NULL))
		{
			minionPrivate->m_castInProgress.reset();
			return EntityState::ID_DEAD;
		}

		const Data::MinionMode* minionMode = minionPublic->m_currentMinionModeId != 0 ? GetManifest()->GetById<Data::MinionMode>(minionPublic->m_currentMinionModeId) : NULL;
		const Components::Position* ownerPosition = ownerEntityInstance->GetComponent<Components::Position>();

		Components::MinionPublic::Command* activeCommand = NULL;
		uint32_t activeCommandPriority = UINT32_MAX;

		minionPrivate->m_targetEntity = SourceEntityInstance();

		bool moveOutOfTheWay = !minionPrivate->m_castInProgress;

		bool isStunned = auras->HasEffect(AuraEffect::ID_STUN, NULL);
		bool isImmobilized = auras->HasEffect(AuraEffect::ID_IMMOBILIZE, NULL);

		const MoveSpeed::Info* moveSpeedInfo = MoveSpeed::GetInfo(combatPublic->m_moveSpeed);
		bool moveOnCooldown = minionPrivate->m_moveCooldownUntilTick + moveSpeedInfo->m_tickBias >= aContext->m_tick;

		if(aEntityState == EntityState::ID_DEFAULT || aEntityState == EntityState::ID_IN_COMBAT)
		{
			if(minionPublic->m_cooldowns.Update(aContext->m_tick))
				minionPublic->SetDirty();

			if (combatPublic->m_interrupt.has_value() && minionPrivate->m_castInProgress.has_value())
			{
				if (combatPublic->m_interrupt->m_cooldownId != 0)
				{
					minionPublic->m_cooldowns.AddCooldown(combatPublic->m_interrupt->m_cooldownId, combatPublic->m_interrupt->m_ticks, aContext->m_tick);
					minionPublic->SetDirty();
				}

				if (minionPrivate->m_castInProgress->m_channeling)
					aContext->m_eventQueue->EventQueueCancelChanneling(aEntityInstanceId, minionPrivate->m_castInProgress->m_targetEntityInstanceId, minionPrivate->m_castInProgress->m_abilityId, minionPrivate->m_castInProgress->m_start);

				minionPrivate->m_castInProgress.reset();
			}

			{
				CastInProgress channeling;
				if (aContext->m_worldView->WorldViewGetChanneling(aEntityInstanceId, channeling))
				{
					minionPrivate->m_castInProgress = channeling;
				}
			}

			if (minionPrivate->m_castInProgress && aContext->m_tick >= minionPrivate->m_castInProgress->m_end)
			{
				if (!minionPrivate->m_castInProgress->m_channeling)
				{
					aContext->m_eventQueue->EventQueueAbility(
						{ aEntityInstanceId, 0 },
						minionPrivate->m_castInProgress->m_targetEntityInstanceId,
						minionPrivate->m_castInProgress->m_aoeTarget,
						GetManifest()->GetById<Data::Ability>(minionPrivate->m_castInProgress->m_abilityId),
						ItemInstanceReference(),
						NULL);
				}
				minionPrivate->m_castInProgress.reset();
			}

			for (Components::MinionPublic::Command& command : minionPublic->m_commands)
			{
				if (command.m_active && !command.m_serverActive)
				{	
					// Start
					command.m_serverActive = true;

					minionPrivate->m_npcMovement.Reset(aContext->m_tick);
				}
				else if(!command.m_active && command.m_serverActive)
				{
					// Stop
					command.m_serverActive = false;
				}

				if(command.m_serverActive)
				{
					const MinionCommand::Info* commandInfo = MinionCommand::GetInfo(command.m_id);
					if(commandInfo->m_priority < activeCommandPriority)
					{
						activeCommand = &command;
						activeCommandPriority = commandInfo->m_priority;
					}
				}
			}

			const Data::Ability* useAbility = NULL;
			uint32_t useAbilityOnEntityInstanceId = 0;

			if(minionPrivate->m_ownerRequestAbility)
			{
				const Components::MinionPrivate::OwnerRequestAbility& ownerRequestAbility = minionPrivate->m_ownerRequestAbility.value();

				if(!isStunned && minionPublic->HasAbility(ownerRequestAbility.m_abilityId) && !minionPrivate->m_castInProgress && !moveOnCooldown)
				{
					const Data::Ability* ability = GetManifest()->GetById<Data::Ability>(ownerRequestAbility.m_abilityId);
					const EntityInstance* ownerRequestAbilityTargetEntityInstance = ownerRequestAbility.m_targetEntityInstanceId != 0 ? aContext->m_worldView->WorldViewSingleEntityInstance(ownerRequestAbility.m_targetEntityInstanceId) : NULL;

					if(ownerRequestAbilityTargetEntityInstance != NULL && !minionPublic->m_cooldowns.IsAbilityOnCooldown(ability))
					{
						const Components::Position* ownerRequestAbilityTargetPosition = ownerRequestAbilityTargetEntityInstance->GetComponent<Components::Position>();
						int32_t distanceSquared = Helpers::CalculateDistanceSquared(ownerRequestAbilityTargetPosition, position);

						if (distanceSquared >= (int32_t)(ability->m_minRange * ability->m_minRange) && distanceSquared <= (int32_t)(ability->m_range * ability->m_range))
						{
							if (combatPublic->HasResourcesForAbility(ability, NULL, combatPublic->GetResourceMax(Resource::ID_MANA)))
							{
								useAbility = ability;
								useAbilityOnEntityInstanceId = ownerRequestAbility.m_targetEntityInstanceId;
							}
						}
					}
				}

				minionPrivate->m_ownerRequestAbility.reset();
			}

			if(activeCommand != NULL)
			{			
				const EntityInstance* targetEntityInstance = activeCommand->m_targetEntityInstanceId != 0 ? aContext->m_worldView->WorldViewSingleEntityInstance(activeCommand->m_targetEntityInstanceId) : NULL;
				const Components::CombatPublic* targetCombatPublic = targetEntityInstance != NULL ? targetEntityInstance->GetComponent<Components::CombatPublic>() : NULL;
				bool targetCanBeAttacked = false;

				if (targetCombatPublic != NULL && (targetEntityInstance->GetState() == EntityState::ID_DEFAULT || targetEntityInstance->GetState() == EntityState::ID_IN_COMBAT))
				{
					if(targetCombatPublic->m_factionId != 0)
					{
						const Data::Faction* faction = GetManifest()->GetById<Data::Faction>(targetCombatPublic->m_factionId);
						if (!faction->IsFriendly())
							targetCanBeAttacked = true;
					}
				}

				switch(activeCommand->m_id)
				{
				case MinionCommand::ID_ATTACK:
					{
						if(targetEntityInstance == NULL || !targetCanBeAttacked)
						{
							activeCommand->m_serverActive = false;
						}
						else if(useAbility == NULL && !isStunned)
						{
							minionPrivate->m_targetEntity = { targetEntityInstance->GetEntityInstanceId(), targetEntityInstance->GetSeq() };

							if(!minionPrivate->m_castInProgress)
							{
								const Components::Position* targetPosition = targetEntityInstance->GetComponent<Components::Position>();
								int32_t distanceSquared = Helpers::CalculateDistanceSquared(targetPosition, position);

								for (uint32_t abilityId : minionPrivate->m_abilityPrio)
								{
									if(minionPublic->IsAbilityBlocked(abilityId))
										continue;

									const Data::Ability* ability = GetManifest()->GetById<Data::Ability>(abilityId);

									if(ability->m_castTime != 0 && moveOnCooldown)
										continue;

									if(ability->ShouldTriggerMoveCooldown() && isImmobilized)
										continue;

									if (ability->TargetFriendly())
									{
										if (minionPublic->m_cooldowns.IsAbilityOnCooldown(ability))
											continue;

										if (!combatPublic->HasResourcesForAbility(ability, NULL, combatPublic->GetResourceMax(Resource::ID_MANA)))
											continue;

										useAbilityOnEntityInstanceId = _PickFriendlyTarget(aContext, aEntityInstanceId, combatPublic, position->m_position, ownerEntityInstance, ability, minionPublic->GetAbility(abilityId), (int32_t)ability->m_range);
										if(useAbilityOnEntityInstanceId == 0)
											continue;

										useAbility = ability;
									}
									else
									{
										if (distanceSquared > (int32_t)(ability->m_range * ability->m_range))
											continue;

										if (distanceSquared < (int32_t)(ability->m_minRange * ability->m_minRange))
											continue;

										if (minionPublic->m_cooldowns.IsAbilityOnCooldown(ability))
											continue;

										if (!combatPublic->HasResourcesForAbility(ability, NULL, combatPublic->GetResourceMax(Resource::ID_MANA)))
											continue;

										if (ability->TargetAOEHostile() || ability->IsOffensive())
										{
											if (ability->TargetAOEHostile() && ability->TargetSelf())
												useAbilityOnEntityInstanceId = aEntityInstanceId;
											else
												useAbilityOnEntityInstanceId = targetEntityInstance->GetEntityInstanceId();

											useAbility = ability;
										}
									}
									break;
								}

								if(useAbility == NULL && distanceSquared > 1)
								{
									moveOutOfTheWay = false;

									// Gotta move closer
									if (!moveOnCooldown)
									{
										if (minionPrivate->m_npcMovement.ShouldResetIfLOS(aContext->m_tick))
										{
											if (aContext->m_worldView->WorldViewIsLineWalkable(position->m_position, targetPosition->m_position))
												minionPrivate->m_npcMovement.Reset(aContext->m_tick);
										}

										IEventQueue::EventQueueMoveRequest moveRequest;
										if (minionPrivate->m_npcMovement.GetMoveRequest(
											aContext->m_worldView->WorldViewGetMapData()->m_mapPathData.get(),
											position->m_position,
											targetPosition->m_position,
											aContext->m_tick,
											position->m_lastMoveTick,
											*aContext->m_random,
											moveRequest))
										{
											moveRequest.m_entityInstanceId = aEntityInstanceId;

											aContext->m_eventQueue->EventQueueMove(moveRequest);

											minionPrivate->m_moveCooldownUntilTick = aContext->m_tick + 2;
										}
									}
								}
							}
						}
					}
					break;

				case MinionCommand::ID_MOVE:
					{
						if(position->m_position == activeCommand->m_targetPosition)
						{
							activeCommand->m_serverActive = false;
						}
						else if(!isImmobilized && !isStunned)
						{
							moveOutOfTheWay = false;
							minionPrivate->m_ownerPositionAtLastMoveCommand = ownerPosition->m_position;

							if (!moveOnCooldown)
							{
								if (minionPrivate->m_npcMovement.ShouldResetIfLOS(aContext->m_tick))
								{
									if (aContext->m_worldView->WorldViewIsLineWalkable(position->m_position, activeCommand->m_targetPosition))
										minionPrivate->m_npcMovement.Reset(aContext->m_tick);
								}

								IEventQueue::EventQueueMoveRequest moveRequest;
								if (minionPrivate->m_npcMovement.GetMoveRequest(
									aContext->m_worldView->WorldViewGetMapData()->m_mapPathData.get(),
									position->m_position,
									activeCommand->m_targetPosition,
									aContext->m_tick,
									position->m_lastMoveTick,
									*aContext->m_random,
									moveRequest))
								{
									moveRequest.m_entityInstanceId = aEntityInstanceId;

									aContext->m_eventQueue->EventQueueMove(moveRequest);

									minionPrivate->m_moveCooldownUntilTick = aContext->m_tick + 2;
								}
							}
						}
					}
					break;

				case MinionCommand::ID_HEAL:
					if(!minionPrivate->m_castInProgress && !isStunned)
					{
						const Data::Ability* healAbility = NULL;
						uint32_t healTargetEntityInstanceId = 0;

						for (const Components::MinionPublic::Ability& minionAbility : minionPublic->m_abilities)
						{
							const Data::Ability* ability = GetManifest()->GetById<Data::Ability>(minionAbility.m_id);
							if (ability->TargetFriendly())
							{
								if (ability->ShouldTriggerMoveCooldown() && isImmobilized)
									continue;

								if (minionPublic->m_cooldowns.IsAbilityOnCooldown(ability))
									continue;

								if (!combatPublic->HasResourcesForAbility(ability, NULL, combatPublic->GetResourceMax(Resource::ID_MANA)))
									continue;

								uint32_t friendlyEntityInstanceId = _PickFriendlyTarget(aContext, aEntityInstanceId, combatPublic, position->m_position, ownerEntityInstance, ability, &minionAbility, (int32_t)ability->m_range * 3);
								if (friendlyEntityInstanceId != 0)
								{
									healAbility = ability;
									healTargetEntityInstanceId = friendlyEntityInstanceId;
									break;
								}
							}
						}

						if(healAbility == NULL)
						{
							activeCommand->m_serverActive = false;
						}
						else if(!isImmobilized)
						{
							const EntityInstance* healTargetEntityInstance = aContext->m_worldView->WorldViewSingleEntityInstance(healTargetEntityInstanceId);
							const Components::Position* healTargetPosition = healTargetEntityInstance->GetComponent<Components::Position>();
							int32_t distanceSquared = healTargetPosition->m_position.DistanceSquared(position->m_position);
							if(distanceSquared > (int32_t)(healAbility->m_range * healAbility->m_range))
							{
								moveOutOfTheWay = false;

								// Move closer
								if (!moveOnCooldown)
								{
									if (minionPrivate->m_npcMovement.ShouldResetIfLOS(aContext->m_tick))
									{
										if (aContext->m_worldView->WorldViewIsLineWalkable(position->m_position, healTargetPosition->m_position))
											minionPrivate->m_npcMovement.Reset(aContext->m_tick);
									}

									IEventQueue::EventQueueMoveRequest moveRequest;
									if (minionPrivate->m_npcMovement.GetMoveRequest(
										aContext->m_worldView->WorldViewGetMapData()->m_mapPathData.get(),
										position->m_position,
										healTargetPosition->m_position,
										aContext->m_tick,
										position->m_lastMoveTick,
										*aContext->m_random,
										moveRequest))
									{
										moveRequest.m_entityInstanceId = aEntityInstanceId;

										aContext->m_eventQueue->EventQueueMove(moveRequest);

										minionPrivate->m_moveCooldownUntilTick = aContext->m_tick + 2;
									}
								}
							}
							else
							{
								if (healAbility->m_castTime == 0 || !moveOnCooldown)
								{
									// Heal
									useAbility = healAbility;
									useAbilityOnEntityInstanceId = healTargetEntityInstanceId;

									activeCommand->m_serverActive = false;
								}
							}
						}
					}
					break;

				default:
					break;
				}
			}
			else
			{
				bool wantsToMove = false;

				if(minionMode != NULL)
				{					
					if(minionMode->m_attackOwnerThreatTarget && minionPublic->m_ownerThreatTargetEntityInstanceId != 0)
					{
						// Something attacked owner - attack it
						Components::MinionPublic::Command* attackCommand = minionPublic->GetCommand(MinionCommand::ID_ATTACK);
						if(attackCommand != NULL)
						{
							attackCommand->m_targetEntityInstanceId = minionPublic->m_ownerThreatTargetEntityInstanceId;
							attackCommand->m_serverActive = true;
						}
					}
					else if(minionMode->m_attackThreatTarget && !threatSource->m_targets.empty())
					{	
						// Something sees us as a threat source - attack it
						Components::MinionPublic::Command* attackCommand = minionPublic->GetCommand(MinionCommand::ID_ATTACK);
						if (attackCommand != NULL)
						{
							attackCommand->m_targetEntityInstanceId = threatSource->GetFirstTarget();
							attackCommand->m_serverActive = true;
						}
					}
					else if (minionMode->m_aggroRange != 0 && aContext->m_tick - minionPrivate->m_lastAggroPingTick >= AGGRO_PING_INTERVAL_TICKS && !isStunned)
					{
						// Attack anything non-friendly within aggro range
						uint32_t aggroEntityInstanceId = 0;

						IWorldView::EntityQuery entityQuery;
						entityQuery.m_position = position->m_position;
						entityQuery.m_maxDistance = (int32_t)minionMode->m_aggroRange;
						entityQuery.m_flags = IWorldView::EntityQuery::FLAG_LINE_OF_SIGHT;

						aContext->m_worldView->WorldViewQueryEntityInstances(entityQuery, [&](
							const EntityInstance* aEntity,
							int32_t				  /*aDistanceSquared*/)
						{
							if (aEntity->GetState() != EntityState::ID_IN_COMBAT && aEntity->GetState() != EntityState::ID_DEFAULT)
								return false;
								
							if(!aEntity->HasComponent<tpublic::Components::NPC>())
								return false;

							const Components::CombatPublic* combatPublic = aEntity->GetComponent<Components::CombatPublic>();
							if(combatPublic == NULL || combatPublic->m_factionId == 0)
								return false;

							const Data::Faction* faction = GetManifest()->GetById<Data::Faction>(combatPublic->m_factionId);
							if(faction->IsFriendly())
								return false;

							aggroEntityInstanceId = aEntity->GetEntityInstanceId();
							return true;
						});

						if(aggroEntityInstanceId != 0)
						{
							Components::MinionPublic::Command* attackCommand = minionPublic->GetCommand(MinionCommand::ID_ATTACK);
							if (attackCommand != NULL)
							{
								attackCommand->m_targetEntityInstanceId = aggroEntityInstanceId;
								attackCommand->m_serverActive = true;
							}
						}

						minionPrivate->m_lastAggroPingTick = aContext->m_tick + (int32_t)((*aContext->m_random)() % 2); // add some jitter
					}					
					else if(useAbility == NULL && !minionPrivate->m_castInProgress && !isStunned)
					{
						for(const Components::MinionPublic::Ability& minionAbility : minionPublic->m_abilities)
						{
							if(!minionPublic->IsAbilityBlocked(minionAbility.m_id))
							{
								const Data::Ability* ability = GetManifest()->GetById<Data::Ability>(minionAbility.m_id);
								if(ability->TargetFriendly())
								{
									if(ability->ShouldTriggerMoveCooldown() && isImmobilized)
										continue;

									if (ability->m_castTime != 0 && moveOnCooldown)
										continue;

									if (minionPublic->m_cooldowns.IsAbilityOnCooldown(ability))
										continue;

									if (!combatPublic->HasResourcesForAbility(ability, NULL, combatPublic->GetResourceMax(Resource::ID_MANA)))
										continue;

									uint32_t friendlyEntityInstanceId = _PickFriendlyTarget(aContext, aEntityInstanceId, combatPublic, position->m_position, ownerEntityInstance, ability, &minionAbility, (int32_t)ability->m_range);
									if (friendlyEntityInstanceId != 0)
									{
										useAbility = ability;
										useAbilityOnEntityInstanceId = friendlyEntityInstanceId;
										break;
									}
								}
							}
						}
					}

					if(useAbility == NULL)
					{
						int32_t ownerDistanceSquared = ownerPosition->m_position.DistanceSquared(position->m_position);
						if(ownerDistanceSquared > OWNER_TELEPORT_DISTANCE * OWNER_TELEPORT_DISTANCE)
						{
							IEventQueue::EventQueueMoveRequest moveRequest;
							moveRequest.m_type = IEventQueue::EventQueueMoveRequest::TYPE_SIMPLE;
							moveRequest.AddToPriorityList(ownerPosition->m_position - position->m_position);
							moveRequest.m_entityInstanceId = aEntityInstanceId;
							moveRequest.m_setTeleportedFlag = true;
							aContext->m_eventQueue->EventQueueMove(moveRequest);

							minionPrivate->m_moveCooldownUntilTick = aContext->m_tick + 2;
						}
						else if(!minionPrivate->m_castInProgress && !isStunned && !isImmobilized &&
							ownerDistanceSquared > (int32_t)(minionMode->m_followDistance * minionMode->m_followDistance) && ownerPosition->m_position != minionPrivate->m_ownerPositionAtLastMoveCommand)
						{
							wantsToMove = true;

							minionPrivate->m_ownerPositionAtLastMoveCommand = Vec2();

							if (!moveOnCooldown)
							{
								if (minionPrivate->m_npcMovement.ShouldResetIfLOS(aContext->m_tick))
								{
									if (aContext->m_worldView->WorldViewIsLineWalkable(position->m_position, ownerPosition->m_position))
										minionPrivate->m_npcMovement.Reset(aContext->m_tick);
								}

								IEventQueue::EventQueueMoveRequest moveRequest;
								if (minionPrivate->m_npcMovement.GetMoveRequest(
									aContext->m_worldView->WorldViewGetMapData()->m_mapPathData.get(),
									position->m_position,
									ownerPosition->m_position,
									aContext->m_tick,
									position->m_lastMoveTick,
									*aContext->m_random,
									moveRequest))
								{
									moveRequest.m_entityInstanceId = aEntityInstanceId;

									aContext->m_eventQueue->EventQueueMove(moveRequest);

									minionPrivate->m_moveCooldownUntilTick = aContext->m_tick + 2;
								}
							}
						}
					}
				}				

				if(!wantsToMove)
					position->m_lastMoveTick = aContext->m_tick;
			}

			if(moveOutOfTheWay && !minionPrivate->m_castInProgress.has_value() && useAbility == NULL && !isImmobilized && !isStunned && !moveOnCooldown)
			{
				bool inTheWay = false;

				aContext->m_worldView->WorldViewEntityInstancesAtPosition(position->m_position, [&](
					const EntityInstance* aEntityInstance)
				{
					if(aEntityInstance->GetEntityInstanceId() != aEntityInstanceId && aEntityInstance->HasComponent<Components::CombatPublic>())
					{
						inTheWay = true;
						return true;
					}
					return false;
				});				

				if(inTheWay)
				{
					IEventQueue::EventQueueMoveRequest moveRequest;						
					moveRequest.AddToPriorityList({ -1, 0 });
					moveRequest.AddToPriorityList({ 1, 0 });
					moveRequest.AddToPriorityList({ 0, -1 });
					moveRequest.AddToPriorityList({ 0, 1 });
					moveRequest.ShufflePriorityList(*aContext->m_random);
					moveRequest.m_entityInstanceId = aEntityInstanceId;
					aContext->m_eventQueue->EventQueueMove(moveRequest);

					minionPrivate->m_moveCooldownUntilTick = aContext->m_tick + 2;
				}
			}

			if (useAbility != NULL)
			{
				position->m_lastMoveTick = aContext->m_tick;
				minionPrivate->m_npcMovement.Reset(aContext->m_tick);

				float cooldownModifier = 0.0f;
				if (useAbility->IsAttack() && useAbility->IsMelee())
					cooldownModifier = auras->GetAttackHaste(GetManifest());

				minionPublic->m_cooldowns.AddAbility(GetManifest(), useAbility, aContext->m_tick, cooldownModifier);
				minionPublic->SetDirty();

				if (useAbility->m_castTime > 0)
				{
					int32_t castTime = useAbility->m_castTime;
					if (useAbility->IsSpell())
						castTime = Haste::CalculateCastTime(castTime, auras->GetSpellHaste(GetManifest()));

					CastInProgress cast;
					cast.m_abilityId = useAbility->m_id;
					cast.m_targetEntityInstanceId = useAbilityOnEntityInstanceId;
					cast.m_start = aContext->m_tick;
					cast.m_end = cast.m_start + castTime;
					minionPrivate->m_castInProgress = cast;
				}
				else
				{
					aContext->m_eventQueue->EventQueueAbility({ aEntityInstanceId, 0 }, useAbilityOnEntityInstanceId, Vec2(), useAbility, ItemInstanceReference(), NULL);
				}
			}		
		}

		{
			bool hadPlayerCombatEvent = false;
			if (combatPublic->m_pvpCombatEvent)
			{
				hadPlayerCombatEvent = true;
				combatPublic->m_pvpCombatEvent = false;
			}

			if (minionPrivate->m_playerCombat)
			{
				if (aContext->m_tick - combatPublic->m_lastPVPCombatEventTick > 5 * 10)
					minionPrivate->m_playerCombat = false;
			}
			else
			{
				if (hadPlayerCombatEvent)
					minionPrivate->m_playerCombat = true;
			}
		}

		if (aEntityState == EntityState::ID_IN_COMBAT)
		{
			if (threatSource->m_targets.size() == 0 && !minionPrivate->m_playerCombat)
				return EntityState::ID_DEFAULT;
		}
		else if (aEntityState == EntityState::ID_DEFAULT)
		{
			if (threatSource->m_targets.size() > 0 || minionPrivate->m_playerCombat)
				return EntityState::ID_IN_COMBAT;
		}

		return EntityState::CONTINUE;
	}

	void
	Minion::UpdatePublic(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			/*aContext*/) 
	{
		const Components::MinionPrivate* minionPrivate = GetComponent<Components::MinionPrivate>(aComponents);

		{
			Components::MinionPublic* minionPublic = GetComponent<Components::MinionPublic>(aComponents);

			bool minionPublicDirty = false;
			bool interactive = aEntityState == EntityState::ID_DEFAULT || aEntityState == EntityState::ID_IN_COMBAT;

			for (Components::MinionPublic::Command& command : minionPublic->m_commands)
			{
				if (!interactive)
					command.m_serverActive = false;

				if (command.m_serverActive != command.m_active)
				{
					command.m_active = command.m_serverActive;

					if (!command.m_active)
					{
						command.m_targetEntityInstanceId = 0;
						command.m_targetPosition = {};
					}

					minionPublicDirty = true;
				}
			}

			if (minionPublicDirty)
				minionPublic->SetDirty();
		}

		{
			Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);
			if (combatPublic->m_targetEntityInstanceId != minionPrivate->m_targetEntity.m_entityInstanceId)
			{
				combatPublic->m_targetEntityInstanceId = minionPrivate->m_targetEntity.m_entityInstanceId;
				combatPublic->SetDirty();
			}

			if (minionPrivate->m_castInProgress.has_value())
			{
				if (combatPublic->m_castInProgress != minionPrivate->m_castInProgress.value())
				{
					combatPublic->m_castInProgress = minionPrivate->m_castInProgress.value();
					combatPublic->SetDirty();
				}
			}
			else
			{
				if (combatPublic->m_castInProgress.has_value())
				{
					combatPublic->m_castInProgress.reset();
					combatPublic->SetDirty();
				}
			}

			combatPublic->m_interrupt.reset();
		}

		{
			Components::Position* position = GetComponent<Components::Position>(aComponents);
			if(position->IsTeleported())
			{
				position->ClearTeleported();
				position->SetDirty();
			}
		}
	}

}