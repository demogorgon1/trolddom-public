#include "../Pcheader.h"

#include <tpublic/Components/Openable.h>
#include <tpublic/Components/PlayerPrivate.h>
#include <tpublic/Components/Position.h>

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/Entity.h>
#include <tpublic/Data/Profession.h>
#include <tpublic/Data/Sprite.h>
#include <tpublic/Data/Zone.h>

#include <tpublic/DirectEffects/Fishing.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Manifest.h>
#include <tpublic/MapData.h>
#include <tpublic/WorldInfoMap.h>

namespace tpublic::DirectEffects
{

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
		const IWorldView*				aWorldView) 
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
					break;
				}
			}
		} 

		if(fishingPosition.has_value())
		{
			uint32_t level = 0;
			uint32_t lootTableId = 0;
			uint32_t triggerAbilityId = 0;
			uint32_t triggerAbilityChance = 0;

			if(map->m_worldInfoMap)
			{
				const WorldInfoMap::Entry& worldInfo = map->m_worldInfoMap->Get(fishingPosition.value());

				level = worldInfo.m_level;

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
				uint32_t skillRequired = (level - 1) * 5;

				const Components::PlayerPrivate* playerPrivate = aTarget->GetComponent<Components::PlayerPrivate>();
				uint32_t fishingProfessionId = aManifest->GetExistingIdByName<Data::Profession>("fishing");

				if(playerPrivate->m_professions.HasProfessionSkill(fishingProfessionId, skillRequired))
				{
					// Each tick has 10% chance of making a splash
					tpublic::UniformDistribution<uint32_t> distribution(1, 100);
					if(distribution(aRandom) <= 10)
					{
						uint32_t fishingSplashEntityId = aManifest->GetExistingIdByName<Data::Entity>("fishing_splash");

						EntityInstance* splashEntity = aEventQueue->EventQueueSpawnEntity(fishingSplashEntityId, EntityState::ID_DEFAULT, 0, false);

						Components::Position* splashPosition = splashEntity->GetComponent<Components::Position>();
						splashPosition->m_position = fishingPosition.value();

						Components::Openable* splashOpenable = splashEntity->GetComponent<Components::Openable>();
						splashOpenable->m_lootTableId = map->m_mapInfo.m_defaultFishingLootTableId;
						splashOpenable->m_requiredProfessionSkill = skillRequired;
						splashOpenable->m_level = level;
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