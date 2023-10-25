#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/Lootable.h>
#include <tpublic/Components/NPC.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/Sprite.h>
#include <tpublic/Components/Tag.h>
#include <tpublic/Components/ThreatTarget.h>

#include <tpublic/Systems/NPC.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Helpers.h>
#include <tpublic/IAbilityQueue.h>
#include <tpublic/IGroupRoundRobin.h>
#include <tpublic/IMoveRequestQueue.h>
#include <tpublic/ItemInstanceData.h>
#include <tpublic/IThreatEventQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/ISystemEventQueue.h>
#include <tpublic/LootGenerator.h>
#include <tpublic/Manifest.h>

namespace tpublic::Systems
{

	NPC::NPC(
		const Manifest*			aManifest)
		: SystemBase(aManifest)
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
		uint32_t				/*aEntityInstanceId*/,
		EntityState::Id			/*aEntityState*/,
		ComponentBase**			aComponents,
		int32_t					/*aTick*/)
	{
		Components::CombatPublic* combat = GetComponent<Components::CombatPublic>(aComponents);
		const Components::NPC* npc = GetComponent<Components::NPC>(aComponents);

		for(const Components::NPC::ResourceEntry& resource : npc->m_resources.m_entries)
		{
			combat->AddResourceMax(resource.m_id, resource.m_max);

			const Resource::Info* info = Resource::GetInfo((Resource::Id)resource.m_id);
			if(info->m_flags & Resource::FLAG_DEFAULT_TO_MAX)
				combat->SetResourceToMax(resource.m_id);
		}

		combat->SetDirty();
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

		const Data::Faction* faction = GetManifest()->m_factions.GetById(combat->m_factionId);

		if (aEntityState != EntityState::ID_DEAD && combat->GetResource(Resource::ID_HEALTH) == 0 && !auras->HasEffect(AuraEffect::ID_IMMORTALITY))
		{
			const Components::Tag* tag = GetComponent<Components::Tag>(aComponents);
			if(tag->m_playerTag.IsSet())
			{
				aContext->m_systemEventQueue->AddKillXPEvent(tag->m_playerTag, combat->m_level);

				Components::Lootable* lootable = GetComponent<Components::Lootable>(aComponents);
				lootable->m_playerTag = tag->m_playerTag;
				lootable->SetDirty();

				if(lootable->m_playerTag.IsSet())
				{
					aContext->m_lootGenerator->Generate(*aContext->m_random, lootable);

					if(lootable->m_availableLoot.size() > 0)
					{
						if(lootable->m_playerTag.IsGroup())
						{
							uint64_t groupId = lootable->m_playerTag.GetGroupId();

							if(tag->m_lootRule == LootRule::ID_FREE_FOR_ALL)
							{
								// Anyone in group can loot
								lootable->SetPlayerTagForAllAvailableLoot(PlayerTag(PlayerTag::TYPE_ANYONE));
							}
							else
							{
								// Group and master loot is decided on per item basis
								for(Components::Lootable::AvailableLoot& loot : lootable->m_availableLoot)
								{
									assert(loot.m_itemInstance.IsSet());
									ItemInstanceData itemInstanceData(GetManifest(), loot.m_itemInstance);
									uint32_t rarity = itemInstanceData.m_properties[Data::Item::PROPERTY_TYPE_RARITY];

									if(rarity < (uint32_t)tag->m_lootThreshold)
									{
										// This item rarity is below the threshold, which means it will be assigned using round-robin
										uint32_t entityInstanceId = aContext->m_groupRoundRobin->GetNextGroupRoundRobinEntityInstanceId(groupId);
										if(entityInstanceId != 0)
										{
											loot.m_playerTag.SetEntityInstanceId(entityInstanceId);
										}										
									}
									else
									{
										// Item is above the rarity threshold, so whoever gets to loot it can't be decided here
										switch(tag->m_lootRule)
										{
										case LootRule::ID_GROUP:
											aContext->m_systemEventQueue->AddGroupLootEvent(aEntityInstanceId, lootable);
											break;

										case LootRule::ID_MASTER:
											loot.m_playerTag = PlayerTag(PlayerTag::TYPE_MASTER);
											break;

										default:
											assert(false);
											break;
										}										
									}
								}
							}
						}						
					}
					else
					{
						// No group
						lootable->SetPlayerTagForAllAvailableLoot(lootable->m_playerTag);
					}
				}
			}

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
						bool isWithinDistance = Helpers::IsWithinDistance(aEntity->GetComponent<Components::Position>(), position, 3);

						if (aEntity->IsPlayer() && (!faction->IsNeutral() || combat->m_targetEntityInstanceId == aEntity->GetEntityInstanceId()))
						{
							if (isWithinDistance)
								aContext->m_threatEventQueue->AddThreatEvent(aEntity->GetEntityInstanceId(), aEntityInstanceId, 0);
						}
						else if(topThreatEntity != NULL && aEntity->GetState() != EntityState::ID_IN_COMBAT && !faction->IsNeutral())
						{
							if (isWithinDistance)
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
				npc->SetDirty();
				returnValue = EntityState::ID_IN_COMBAT;
			}
			break;

		case EntityState::ID_IN_COMBAT:
			{
				if (threat->m_table.IsEmpty())
				{
					npc->m_targetEntityInstanceId = 0;
					npc->m_castInProgress.reset();

					Components::Tag* tag = GetComponent<Components::Tag>(aComponents);
					tag->m_playerTag.Clear();
					tag->SetDirty();

					returnValue = EntityState::ID_DEFAULT;
				}
				else if(auras->HasEffect(AuraEffect::ID_STUN))
				{
					npc->m_castInProgress.reset();
				}
				else
				{
					uint32_t topThreatEntityInstanceId = threat->m_table.GetTop()->m_entityInstanceId;
					npc->m_targetEntityInstanceId = topThreatEntityInstanceId;
					
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

							if (distanceSquared > (int32_t)(ability->m_range * ability->m_range))
								continue;
								
							if(npc->m_cooldowns.Get(ability->m_id) != NULL)
								continue;

							if(!combat->HasResourcesForAbility(ability))
								continue;

							if (abilityEntry.m_useProbability == UINT32_MAX || (*aContext->m_random)() < abilityEntry.m_useProbability)
							{
								useAbility = ability;
								break;
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

		bool block = (aEntityState != EntityState::ID_DEAD && aEntityState != EntityState::ID_DESPAWNING);

		if(position->m_block != block)
		{
			position->m_block = block;
			position->SetDirty();
		}
	}

}

