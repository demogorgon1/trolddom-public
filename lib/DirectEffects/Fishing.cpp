#include "../Pcheader.h"

#include <tpublic/Components/NPC.h>
#include <tpublic/Components/Openable.h>
#include <tpublic/Components/PlayerPrivate.h>
#include <tpublic/Components/Position.h>

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/Entity.h>
#include <tpublic/Data/LootTable.h>
#include <tpublic/Data/MapEntitySpawn.h>
#include <tpublic/Data/Profession.h>
#include <tpublic/Data/Sprite.h>
#include <tpublic/Data/Zone.h>

#include <tpublic/DirectEffects/Fishing.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Manifest.h>
#include <tpublic/MapData.h>
#include <tpublic/Requirements.h>
#include <tpublic/WorldInfoMap.h>

namespace tpublic::DirectEffects
{

	namespace
	{

		// Sometimes a monster takes the bait instead of a fish (with a splash that can't be caught): picked from the map's
		// fishing map entity spawn (by level, zone and tile flags of the fishing tile, like any other map entity spawn) and
		// attacking the fisher right away
		bool
		_SpawnMonster(
			std::mt19937&					aRandom,
			const Manifest*					aManifest,
			const MapData*					aMap,
			uint32_t						aLevel,
			uint32_t						aZoneId,
			uint32_t						aTileFlags,
			const Vec2&						aPosition,
			EntityInstance*					aFisher,
			IEventQueue*					aEventQueue)
		{
			const MapInfo& mapInfo = aMap->m_mapInfo;
			if(mapInfo.m_defaultFishingMapEntitySpawnId == 0 || mapInfo.m_fishingMonsterChance == 0)
				return false;

			UniformDistribution<uint32_t> chance(1, 1000);
			if(chance(aRandom) > mapInfo.m_fishingMonsterChance)
				return false;

			const Data::MapEntitySpawn* mapEntitySpawn = aManifest->GetById<Data::MapEntitySpawn>(mapInfo.m_defaultFishingMapEntitySpawnId);

			std::vector<const Data::MapEntitySpawn::Entity*> possibilities;
			uint32_t totalWeight = 0;
			for(const std::unique_ptr<Data::MapEntitySpawn::Entity>& entity : mapEntitySpawn->m_entities)
			{
				if(entity->m_weight > 0 && entity->HasLevel(aLevel) && entity->HasZone(aZoneId) && entity->CheckTileFlags(aTileFlags))
				{
					possibilities.push_back(entity.get());
					totalWeight += entity->m_weight;
				}
			}

			if(totalWeight == 0)
				return false;

			const Data::MapEntitySpawn::Entity* selected = NULL;
			{
				UniformDistribution<uint32_t> distribution(1, totalWeight);
				uint32_t roll = distribution(aRandom);
				for(const Data::MapEntitySpawn::Entity* possibility : possibilities)
				{
					if(roll <= possibility->m_weight)
					{
						selected = possibility;
						break;
					}
					roll -= possibility->m_weight;
				}
				assert(selected != NULL);
			}

			EntityInstance* monster = aEventQueue->EventQueueSpawnEntity(selected->m_entityId, selected->m_initState, 0, false);

			monster->GetComponent<Components::Position>()->m_position = aPosition;

			Components::NPC* npc = monster->GetComponent<Components::NPC>();
			if(npc != NULL)
				npc->m_spawnWithTarget = { { aFisher->GetEntityInstanceId(), aFisher->GetSeq() }, 1 };

			// Splash where it came out of the water, with nothing to catch
			uint32_t splashEntityId = aManifest->GetExistingIdByName<Data::Entity>("fishing_monster_splash");
			EntityInstance* splash = aEventQueue->EventQueueSpawnEntity(splashEntityId, EntityState::ID_DEFAULT, 0, false);
			splash->GetComponent<Components::Position>()->m_position = aPosition;

			return true;
		}

	}

	void
	Fishing::FromSource(
		const SourceNode*				/*aSource*/) 
	{
	}

	void
	Fishing::ToStream(
		IWriter*						aStream) const 
	{
		ToStreamBase(aStream);
	}

	bool
	Fishing::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))	
			return false;
		return true;
	}

	DirectEffectBase::Result
	Fishing::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					aRandom,
		const Manifest*					aManifest,
		CombatEvent::Id					/*aId*/,
		uint32_t						/*aAbilityId*/,
		const SourceEntityInstance&		aSourceEntityInstance,
		EntityInstance*					aSource,
		EntityInstance*					aTarget,
		const Vec2&						/*aAOETarget*/,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			/*aCombatResultQueue*/,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					aEventQueue,
		const IWorldView*				aWorldView,
		bool							/*aOffHand*/)
	{		
		const tpublic::Components::Position* position = aTarget->GetComponent<tpublic::Components::Position>();
		const tpublic::MapData* map = aWorldView->WorldViewGetMapData();

		static const Vec2 NEIGHBORS[8] = 
		{
			{ 1, 1 },
			{ -1, -1 },
			{ 1, -1 },
			{ -1, 1 },
			{ 1, 0 },
			{ 0, 1 },
			{ 0, -1 },
			{ -1, 0 }
		};

		std::optional<Vec2> fishingPosition;
		uint32_t fishingTileFlags = 0;

		uint32_t startOffset = aRandom() % 8;
		for(uint32_t i = 0; i < 8; i++)
		{
			uint32_t j = (i + startOffset) % 8;
			Vec2 p = position->m_position + NEIGHBORS[j];
			uint32_t tile = map->GetTile(p);
			if(tile != 0)
			{
				const Data::Sprite* tileSpriteData = aManifest->GetById<Data::Sprite>(tile);
				if(tileSpriteData->m_info.m_flags & SpriteInfo::FLAG_TILE_FISHABLE)
				{
					fishingPosition = p;
					fishingTileFlags = tileSpriteData->m_info.m_flags;
					break;
				}
			}
		} 

		if(fishingPosition.has_value())
		{
			uint32_t level = 0;
			uint32_t zoneId = 0;
			uint32_t lootTableId = 0;
			uint32_t triggerAbilityId = 0;
			uint32_t triggerAbilityChance = 0;

			if(map->m_worldInfoMap)
			{
				const WorldInfoMap::Entry& worldInfo = map->m_worldInfoMap->Get(fishingPosition.value());

				level = worldInfo.m_level;
				zoneId = worldInfo.m_zoneId;

				const Data::Zone* zone = worldInfo.m_zoneId != 0 ? aManifest->GetById<Data::Zone>(worldInfo.m_zoneId) : NULL;
				const Data::Zone* subZone = worldInfo.m_subZoneId != 0 ? aManifest->GetById<Data::Zone>(worldInfo.m_subZoneId) : NULL;

				if(subZone != NULL && subZone->m_fishingLootTableId != 0)
					lootTableId = subZone->m_fishingLootTableId;
				else if (zone != NULL && zone->m_fishingLootTableId != 0)
					lootTableId = zone->m_fishingLootTableId;

				if (subZone != NULL && subZone->m_fishingTriggerAbilityId != 0)
				{
					triggerAbilityId = subZone->m_fishingTriggerAbilityId;
					triggerAbilityChance = subZone->m_fishingTriggerAbilityChance;
				}
				else if (zone != NULL && zone->m_fishingTriggerAbilityId != 0)
				{
					triggerAbilityId = zone->m_fishingTriggerAbilityId;
					triggerAbilityChance = zone->m_fishingTriggerAbilityChance;
				}
			}
			else 
			{
				level = map->m_mapInfo.m_level;
			}

			if(level == 0)
				level = 1;

			if(lootTableId == 0)
				lootTableId = map->m_mapInfo.m_defaultFishingLootTableId;

			if(lootTableId != 0)
			{				
				bool hasPossibleLoot = false;

				// Check all loot possibilities... only proceed (make splash) if all requirements are matched for at least one possibibility
				{
					const Data::LootTable* lootTable = aManifest->GetById<Data::LootTable>(lootTableId);
					for (const std::unique_ptr<Data::LootTable::Slot>& slot : lootTable->m_slots)
					{
						for (const Data::LootTable::Possibility& possibility : slot->m_possibilities)
						{
							if (Requirements::CheckList(aManifest, possibility.m_requirements, aWorldView, aSource, aTarget))
							{
								hasPossibleLoot = true;
								break;
							}
						}

						if (hasPossibleLoot)
							break;
					}
				}

				if(hasPossibleLoot)
				{
					uint32_t skillRequired = (level - 1) * 5;

					const Components::PlayerPrivate* playerPrivate = aTarget->GetComponent<Components::PlayerPrivate>();
					uint32_t fishingProfessionId = aManifest->GetExistingIdByName<Data::Profession>("fishing");

					if (playerPrivate->m_professions.HasProfessionSkill(fishingProfessionId, skillRequired))
					{
						// Each tick has 15% chance of making a splash
						UniformDistribution<uint32_t> distribution(1, 100);
						if (distribution(aRandom) <= 15 && !_SpawnMonster(aRandom, aManifest, map, level, zoneId, fishingTileFlags, fishingPosition.value(), aTarget, aEventQueue))
						{
							uint32_t fishingSplashEntityId = aManifest->GetExistingIdByName<Data::Entity>("fishing_splash");

							EntityInstance* splashEntity = aEventQueue->EventQueueSpawnEntity(fishingSplashEntityId, EntityState::ID_DEFAULT, 0, false);

							Components::Position* splashPosition = splashEntity->GetComponent<Components::Position>();
							splashPosition->m_position = fishingPosition.value();

							Components::Openable* splashOpenable = splashEntity->GetComponent<Components::Openable>();
							splashOpenable->m_lootTableId = lootTableId;
							splashOpenable->m_requiredProfessionSkill = skillRequired;
							splashOpenable->m_level = level;
							splashOpenable->m_ownerEntityInstanceId = aSource->GetEntityInstanceId();
						}
					}
				}
			}

			if(triggerAbilityId != 0 && triggerAbilityChance > 0)
			{
				tpublic::UniformDistribution<uint32_t> distribution(1, 100);
				if (distribution(aRandom) <= triggerAbilityChance)
				{
					const Data::Ability* triggerAbility = aManifest->GetById<Data::Ability>(triggerAbilityId);

					aEventQueue->EventQueueAbility(aSourceEntityInstance, aTarget->GetEntityInstanceId(), Vec2(), triggerAbility);
				}
			}
		}
		else if(aSource != NULL)
		{
			aEventQueue->EventQueueInterrupt(aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId(), 0, 0); 
		}

		return Result();
	}

}