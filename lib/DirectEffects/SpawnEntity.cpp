#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/DisplayName.h>
#include <tpublic/Components/GuildName.h>
#include <tpublic/Components/MinionPrivate.h>
#include <tpublic/Components/MinionPublic.h>
#include <tpublic/Components/NPC.h>
#include <tpublic/Components/Owner.h>
#include <tpublic/Components/PlayerPublic.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/ThreatTarget.h>

#include <tpublic/Data/Entity.h>

#include <tpublic/DirectEffects/SpawnEntity.h>

#include <tpublic/ApplyNPCMetrics.h>
#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Manifest.h>

namespace tpublic::DirectEffects
{

	void
	SpawnEntity::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if (aChild->m_name == "entity")
				{
					m_entityIds.push_back(aChild->GetId(DataType::ID_ENTITY));
				}
				else if (aChild->m_name == "entities")
				{
					aChild->GetIdArray(DataType::ID_ENTITY, m_entityIds);
				}
				else if (aChild->m_name == "map_entity_spawn")
				{
					m_mapEntitySpawnId = aChild->GetId(DataType::ID_MAP_ENTITY_SPAWN);
				}
				else if (aChild->m_name == "spawn_flags")
				{
					m_spawnFlags = SourceToSpawnFlags(aChild);
				}
				else if (aChild->m_name == "refresh_npc_metrics")
				{
					m_refreshNPCMetrics = RefreshNPCMetrics(aChild);
				}
				else if (aChild->m_name == "npc_target_threat")
				{
					m_npcTargetThreat = aChild->GetInt32();
				}
				else if (aChild->m_name == "init_state")
				{
					m_initState = EntityState::StringToId(aChild->GetIdentifier());
					TP_VERIFY(m_initState != EntityState::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid entity state.", aChild->GetIdentifier());
				}
				else if(aChild->m_name == "must_have_one_at_target")
				{
					aChild->GetArray()->ForEachChild([&](
						const SourceNode* aChild)
					{
						m_mustHaveOneAtTarget.push_back(MustHaveOneAtTarget(aChild));
					});
				}
				else if(aChild->m_name == "aura_conditional")
				{
					m_auraConditionals.push_back(std::make_unique<AuraConditional>(aChild));
				}
				else
				{
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
				}
			}
		});
	}

	void
	SpawnEntity::ToStream(
		IWriter*						aStream) const 
	{	
		ToStreamBase(aStream);
		aStream->WriteUInt(m_mapEntitySpawnId);
		aStream->WriteUInts(m_entityIds);
		aStream->WritePOD(m_spawnFlags);
		aStream->WriteInt(m_npcTargetThreat);
		aStream->WritePOD(m_initState);
		aStream->WriteOptionalObject(m_refreshNPCMetrics);
		aStream->WriteObjects(m_mustHaveOneAtTarget);
		aStream->WriteObjectPointers(m_auraConditionals);
	}

	bool
	SpawnEntity::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		if (!aStream->ReadUInt(m_mapEntitySpawnId))
			return false;
		if (!aStream->ReadUInts(m_entityIds))
			return false;
		if (!aStream->ReadPOD(m_spawnFlags))
			return false;
		if (!aStream->ReadInt(m_npcTargetThreat))
			return false;
		if (!aStream->ReadPOD(m_initState))
			return false;
		if(!aStream->ReadOptionalObject(m_refreshNPCMetrics))
			return false;
		if(!aStream->ReadObjects(m_mustHaveOneAtTarget))
			return false;
		if(!aStream->ReadObjectPointers(m_auraConditionals))
			return false;
		return true;
	}

	DirectEffectBase::Result
	SpawnEntity::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					aRandom,
		const Manifest*					aManifest,
		CombatEvent::Id					/*aId*/,
		uint32_t						/*aAbilityId*/,
		const SourceEntityInstance&		/*aSourceEntityInstance*/,
		EntityInstance*					aSource,
		EntityInstance*					aTarget,
		const Vec2&						aAOETarget,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			/*aCombatResultQueue*/,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					aEventQueue,
		const IWorldView*				aWorldView,
		bool							/*aOffHand*/)
	{					
		if(aSource == NULL)
			return Result();

		if(m_mustHaveOneAtTarget.size() > 0)
		{
			std::vector<const EntityInstance*> entitiesAtTarget;

			aWorldView->WorldViewEntityInstancesAtPosition(aAOETarget, [&](
				const EntityInstance* aEntityInstance)
			{	
				entitiesAtTarget.push_back(aEntityInstance);
				return false;
			});

			bool hasOne = false;

			for(const MustHaveOneAtTarget& mustHaveOneAtTarget : m_mustHaveOneAtTarget)
			{
				bool ok = false;

				for(const EntityInstance* entityAtTarget : entitiesAtTarget)
				{
					const Components::CombatPublic* combatPublic = entityAtTarget->GetComponent<Components::CombatPublic>();

					ok = 
						(mustHaveOneAtTarget.m_creatureTypeIds.size() == 0 || (combatPublic != NULL && combatPublic->IsOneOfCreatureTypes(mustHaveOneAtTarget.m_creatureTypeIds))) &&
						(mustHaveOneAtTarget.m_entityIds.size() == 0 || entityAtTarget->IsEntity(mustHaveOneAtTarget.m_entityIds)) &&
						(mustHaveOneAtTarget.m_entityStateIds.size() == 0 || entityAtTarget->IsInState(mustHaveOneAtTarget.m_entityStateIds));

					if(ok)
						break;
				}

				if(ok)
				{
					hasOne = true;
					break;
				}
			}

			if(!hasOne)
				return Result();
		}

		const RefreshNPCMetrics* refreshNPCMetrics = m_refreshNPCMetrics.has_value() ? &m_refreshNPCMetrics.value() : NULL;

		bool detached = (m_spawnFlags & SPAWN_FLAG_DETACHED) != 0;
		bool baseOnNPC = false;
		uint32_t lootTableId = 0;

		EntityInstance* spawnedEntity = NULL;
		
		if(!m_entityIds.empty())
		{
			spawnedEntity = aEventQueue->EventQueueSpawnEntity(Helpers::RandomItem(aRandom, m_entityIds), m_initState, m_mapEntitySpawnId, detached);
		}
		else if (!m_auraConditionals.empty())
		{
			for (const std::unique_ptr<AuraConditional>& auraConditional : m_auraConditionals)
			{
				if(auraConditional->m_auraId == 0 || auraConditional->m_entities.empty())
					continue;

				const EntityInstance* entity = aSource;
				if (auraConditional->m_owner)
				{
					const Components::Owner* owner = entity->GetComponent<Components::Owner>();
					if (owner != NULL && owner->m_ownerSourceEntityInstance.IsSet())
						entity = aWorldView->WorldViewSingleEntityInstance(owner->m_ownerSourceEntityInstance.m_entityInstanceId);
					else
						entity = NULL;
				}

				if (entity != NULL)
				{
					const Components::Auras* auras = entity->GetComponent<Components::Auras>();
					if (auras != NULL)
					{
						const Components::Auras::Entry* auraEntry = auras->GetAura(auraConditional->m_auraId);

						bool ok = auras != NULL;

						if (ok && auraConditional->m_exactCharges != 0 && auraEntry->m_charges != auraConditional->m_exactCharges)
							ok = false;

						if (ok)
						{
							const AuraConditionalEntity* possibility = Helpers::RandomItemPointer(aRandom, auraConditional->m_entities);
							assert(possibility != NULL);

							spawnedEntity = aEventQueue->EventQueueSpawnEntity(possibility->m_entityId, m_initState, m_mapEntitySpawnId, detached);

							if(auraConditional->m_consume)
								aEventQueue->EventQueueRemoveAura(entity->GetEntityInstanceId(), auraConditional->m_auraId);

							if(possibility->m_refreshNPCMetrics.has_value())
								refreshNPCMetrics = &possibility->m_refreshNPCMetrics.value();
							else if(auraConditional->m_refreshNPCMetrics.has_value())
								refreshNPCMetrics = &auraConditional->m_refreshNPCMetrics.value();

							lootTableId = auraConditional->m_lootTableId;

							baseOnNPC = true;
						}
					}
				}

				if (spawnedEntity != NULL)
					break;
			}
		}
		
		if(spawnedEntity == NULL)
			return Result();

		if(m_mapEntitySpawnId == 0)
		{
			// Position
			Components::Position* spawnedEntityPosition = spawnedEntity->GetComponent<Components::Position>();

			if (m_spawnFlags & SPAWN_FLAG_AT_TARGET)
				spawnedEntityPosition->m_position = aTarget->GetComponent<Components::Position>()->m_position;
			else if (m_spawnFlags & SPAWN_FLAG_AT_AOE_TARGET)
				spawnedEntityPosition->m_position = aAOETarget;
			else
				spawnedEntityPosition->m_position = aSource->GetComponent<Components::Position>()->m_position;

			if (m_spawnFlags & SPAWN_FLAG_NEARBY)
				spawnedEntityPosition->m_position = aWorldView->WorldViewGetNearbyFreePosition(spawnedEntityPosition->m_position, 1, aRandom);
		}

		if((m_spawnFlags & SPAWN_FLAG_NO_OWNER) == 0)
		{
			// Owner
			Components::Owner* spawnedEntityOwner = spawnedEntity->GetComponent<Components::Owner>();
			spawnedEntityOwner->m_ownerSourceEntityInstance = { aSource->GetEntityInstanceId(), aSource->GetSeq() };
		}

		if(m_npcTargetThreat != 0)
		{
			// Initial NPC threat on target
			Components::NPC* npc = spawnedEntity->GetComponent<Components::NPC>();

			if ((m_spawnFlags & SPAWN_FLAG_SOURCE_THREAT_TARGET) != 0)
			{
				const Components::ThreatTarget* sourceThreatTarget = aSource->GetComponent<Components::ThreatTarget>();

				if (sourceThreatTarget != NULL)
				{
					const ThreatTable::Entry* topThreat = sourceThreatTarget->m_table.GetTop();
					if (topThreat != NULL)
						npc->m_spawnWithTarget = { topThreat->m_key, m_npcTargetThreat };
				}
			}
			else if((m_spawnFlags & SPAWN_FLAG_SOURCE_THREAT) != 0)
			{
				npc->m_spawnWithTarget = { { aSource->GetEntityInstanceId(), aSource->GetSeq() }, m_npcTargetThreat };
			}
			else
			{
				npc->m_spawnWithTarget = { { aTarget->GetEntityInstanceId(), aTarget->GetSeq() }, m_npcTargetThreat };
			}
		}

		// Minion?
		{
			Components::MinionPublic* minionPublic = spawnedEntity->GetComponent<Components::MinionPublic>();
			if(minionPublic != NULL)
			{
				if(m_spawnFlags & SPAWN_FLAG_SOURCE_LEVEL)				
				{
					Components::CombatPublic* sourceCombatPublic = aSource->GetComponent<Components::CombatPublic>();
					Components::CombatPublic* spawnedCombatPublic = spawnedEntity->GetComponent<Components::CombatPublic>();
					if (sourceCombatPublic != NULL && spawnedCombatPublic != NULL)
						spawnedCombatPublic->m_level = sourceCombatPublic->m_level;
				}

				if(minionPublic->m_durationSeconds != 0)
					minionPublic->m_spawnTimeStamp = (uint64_t)time(NULL);
				
				minionPublic->m_ownerEntityInstanceId = aSource->GetEntityInstanceId();
				minionPublic->SetDirty();
			}

			Components::MinionPrivate* minionPrivate = spawnedEntity->GetComponent<Components::MinionPrivate>();
			if (minionPrivate != NULL && aSource->IsPlayer())
			{
				const Components::PlayerPublic* playerPublic = aSource->GetComponent<Components::PlayerPublic>();

				if(minionPrivate->m_seed == 0) // No minion seed, derive one from the spawner's character id and minion entity id
					minionPrivate->m_seed = (uint32_t)Hash::Splitmix_2_32(playerPublic->m_characterId, spawnedEntity->GetEntityId());

				minionPrivate->SetDirty();

				// Guild name should show owner
				const Components::DisplayName* displayName = aSource->GetComponent<Components::DisplayName>();
				Components::GuildName* guildName = spawnedEntity->GetComponent<Components::GuildName>();
				guildName->m_string = displayName->m_string + "'s Minion";
				guildName->SetDirty();
			}
		}

		// Refresh NPC metrics?
		if(refreshNPCMetrics != NULL)
		{
			Data::Entity::Modifiers modifiers;
			modifiers.m_weaponDamage = refreshNPCMetrics->m_weaponDamage;
			modifiers.m_armor = refreshNPCMetrics->m_armor;
			modifiers.m_resources[Resource::ID_HEALTH] = refreshNPCMetrics->m_health;

			Components::CombatPublic* combatPublic = spawnedEntity->GetComponent<Components::CombatPublic>();
			Components::CombatPrivate* combatPrivate = spawnedEntity->GetComponent<Components::CombatPrivate>();
			Components::MinionPrivate* minionPrivate = spawnedEntity->GetComponent<Components::MinionPrivate>();
			Components::MinionPublic* minionPublic = spawnedEntity->GetComponent<Components::MinionPublic>();
			Components::NPC* npc = spawnedEntity->GetComponent<Components::NPC>();

			if(refreshNPCMetrics->m_elite)
			{	
				if(refreshNPCMetrics->m_elite.value())
					combatPublic->m_combatFlags |= Components::CombatPublic::COMBAT_FLAG_ELITE;
				else
					combatPublic->m_combatFlags &= ~Components::CombatPublic::COMBAT_FLAG_ELITE;
			}

			ApplyNPCMetrics::Process(
				aManifest->m_npcMetrics.get(),
				modifiers,
				combatPublic,
				combatPrivate,
				minionPrivate,
				npc,
				refreshNPCMetrics->m_update,
				baseOnNPC);

			if(minionPublic != NULL)
				minionPublic->m_toolTipWeaponDamageBase = UIntRange(combatPrivate->m_weaponDamageRangeMin, combatPrivate->m_weaponDamageRangeMax);
		}

		// Force loot table?
		if(lootTableId != 0)
		{
			Components::Lootable* lootable = spawnedEntity->GetComponent<Components::Lootable>();
			if(lootable != NULL)
				lootable->m_lootTableId = lootTableId;
		}

		if(m_spawnFlags & SPAWN_FLAG_DESPAWN_SOURCE)
		{
			Result result;
			result.m_despawnSource = true;
			return result;
		}
		
		return Result();
	}

}