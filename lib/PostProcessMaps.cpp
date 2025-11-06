#include "Pcheader.h"

#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/Lootable.h>
#include <tpublic/Components/Openable.h>

#include <tpublic/Data/DialogueRoot.h>
#include <tpublic/Data/DialogueScreen.h>
#include <tpublic/Data/Entity.h>
#include <tpublic/Data/Item.h>
#include <tpublic/Data/LootGroup.h>
#include <tpublic/Data/LootTable.h>
#include <tpublic/Data/Map.h>
#include <tpublic/Data/MapEntitySpawn.h>
#include <tpublic/Data/Tag.h>
#include <tpublic/Data/Zone.h>

#include <tpublic/LootGenerator.h>
#include <tpublic/Manifest.h>
#include <tpublic/WorldInfoMap.h>

namespace tpublic
{
	
	namespace
	{		

		struct ItemMap
		{
			Data::Item::Oracle*
			GetItemOracle(
				uint32_t					aItemId)
			{
				Table::iterator i = m_table.find(aItemId);
				if(i == m_table.cend())
					return NULL;

				Data::Item* t = i->second;

				if(!t->m_oracle)
					t->m_oracle = std::make_unique<Data::Item::Oracle>();

				return t->m_oracle.get();
			}

			void
			Build(
				Manifest*					aManifest)
			{
				aManifest->GetContainer<Data::Item>()->ForEach([&](
					Data::Item* aItem) -> bool
				{
					m_table[aItem->m_id] = aItem;
					return true; // Continue
				});				

				m_lootGenerator = std::make_unique<LootGenerator>(aManifest);
			}

			// Public data
			typedef std::unordered_map<uint32_t, Data::Item*> Table;
			Table								m_table;
			std::unique_ptr<LootGenerator>		m_lootGenerator;
		};

		struct Loot
		{
			uint32_t							m_lootGroupId = 0;
			uint32_t							m_itemId = 0;
		};

		void	
		_GetPossibleQuests(
			const Manifest*					aManifest, 
			uint32_t						aDialogueScreenId,
			std::unordered_set<uint32_t>&	aVisited,
			std::vector<uint32_t>&			aOutQuestIds)
		{
			aVisited.insert(aDialogueScreenId);

			const Data::DialogueScreen* dialogueScreen = aManifest->GetById<Data::DialogueScreen>(aDialogueScreenId);
			for(const Data::DialogueScreen::Option& option : dialogueScreen->m_options)
			{
				if(option.m_questId != 0 && !option.m_noPointOfInterest && option.m_questCompletion) // FIXME: only quest completion for now
					aOutQuestIds.push_back(option.m_questId);
				else if(option.m_dialogueScreenId != 0 && !aVisited.contains(option.m_dialogueScreenId))
					_GetPossibleQuests(aManifest, option.m_dialogueScreenId, aVisited, aOutQuestIds);
			}
		}

		void
		_GetPossibleLoot(
			const Manifest*								aManifest,
			uint32_t									aLevel,
			uint32_t									aLootTableId,
			uint32_t									aCreatureTypeId,
			bool										aElite,
			uint32_t									aTagId,
			ItemMap&									aItemMap,
			std::vector<Loot>&							aOutLoot)
		{
			if(aLootTableId == 0)
				return;

			const Data::LootTable* lootTable = aManifest->GetById<Data::LootTable>(aLootTableId);

			for(const std::unique_ptr<Data::LootTable::Slot>& slot : lootTable->m_slots)
			{
				for(const Data::LootTable::Possibility& possibility : slot->m_possibilities)
				{
					if(!possibility.HasCreatureType(aCreatureTypeId))
						continue;

					if(aElite && possibility.m_elite == Data::LootTable::Possibility::ELITE_MUST_NOT_BE)
						continue;

					if (!aElite && possibility.m_elite == Data::LootTable::Possibility::ELITE_MUST_BE)
						continue;

					if(possibility.m_lootGroupId == 0)
						continue;

					aItemMap.m_lootGenerator->QueryLootGroup(possibility.m_lootGroupId, aLevel, [&](
						const ItemInstance& aItemInstance,
						uint32_t			/*aLootCooldownId*/)
					{
						const Data::Item* item = aManifest->GetById<Data::Item>(aItemInstance.m_itemId);
						if(aTagId == 0 || item->HasTag(aTagId))
							aOutLoot.push_back({ possibility.m_lootGroupId, item->m_id });
					});
				}
			}
		}
			
		void
		_EntityAtPosition(
			const Manifest*								aManifest,
			const Vec2&									aPosition,
			uint32_t									aEntityId,
			Data::Map*									aMap,
			ItemMap&									aItemMap,
			std::unordered_set<std::string>&			aOutErrors)
		{
			const Data::Entity* entity = aManifest->GetById<Data::Entity>(aEntityId);
			const Components::CombatPublic* combatPublic = entity->TryGetComponent<Components::CombatPublic>();

			// Quest giver?
			{
				if (combatPublic != NULL && combatPublic->m_dialogueRootId != 0)
				{
					const Data::DialogueRoot* dialogueRoot = aManifest->GetById<Data::DialogueRoot>(combatPublic->m_dialogueRootId);

					std::vector<uint32_t> possibleQuestIds;
					std::unordered_set<uint32_t> visited;

					for (const std::unique_ptr<Data::DialogueRoot::Entry>& entry : dialogueRoot->m_entries)
						_GetPossibleQuests(aManifest, entry->m_dialogueScreenId, visited, possibleQuestIds);

					if (possibleQuestIds.size() > 0)
					{
						MapData::PointOfInterest t;
						t.m_type = MapData::POINT_OF_INTEREST_TYPE_QUEST_GIVER;
						t.m_position = aPosition;
						t.m_questIds = std::move(possibleQuestIds);
						t.m_dialogueRootId = combatPublic->m_dialogueRootId;
						t.m_entityId = aEntityId;
						aMap->m_data->m_pointsOfInterest.push_back(t);
					}
				}
			}

			// Item oracle info?
			if (aMap->m_data->m_worldInfoMap && !entity->m_displayName.empty())
			{
				const Components::Lootable* lootable = entity->TryGetComponent<Components::Lootable>();

				const WorldInfoMap::Entry& worldInfoMapEntry = aMap->m_data->m_worldInfoMap->Get(aPosition);
				const Data::Zone* zone = worldInfoMapEntry.m_zoneId != 0 ? aManifest->GetById<Data::Zone>(worldInfoMapEntry.m_zoneId) : NULL;

				while (zone != NULL && zone->m_topZoneId != 0)
					zone = aManifest->GetById<Data::Zone>(zone->m_topZoneId);

				uint32_t reagentTagId = aManifest->GetExistingIdByName<Data::Tag>("reagent");

				if(lootable != NULL && lootable->m_lootTableId != 0)
				{
					uint32_t creatureTypeId = combatPublic != NULL ? combatPublic->m_creatureTypeId : 0;
					bool elite = false;
					uint32_t level = aMap->m_data->m_mapInfo.m_level;

					if (combatPublic != NULL)
					{
						elite = combatPublic->IsElite();
						level = combatPublic->m_level;
					}
					else
					{
						elite = aMap->m_data->m_mapInfo.m_openableElite && entity->TryGetComponent<Components::Openable>();
						level = worldInfoMapEntry.m_level;
					}

					if(level != 0)
					{
						std::vector<Loot> loot;

						_GetPossibleLoot(aManifest, level, lootable->m_lootTableId, creatureTypeId, elite, reagentTagId, aItemMap, loot);

						for (uint32_t mapLootTableId : aMap->m_data->m_mapInfo.m_mapLootTableIds)
							_GetPossibleLoot(aManifest, level, mapLootTableId, creatureTypeId, elite, reagentTagId, aItemMap, loot);

						if (worldInfoMapEntry.m_zoneId != 0)
							_GetPossibleLoot(aManifest, level, aManifest->GetById<Data::Zone>(worldInfoMapEntry.m_zoneId)->m_lootTableId, creatureTypeId, elite, reagentTagId, aItemMap, loot);

						if (worldInfoMapEntry.m_subZoneId != 0)
							_GetPossibleLoot(aManifest, level, aManifest->GetById<Data::Zone>(worldInfoMapEntry.m_subZoneId)->m_lootTableId, creatureTypeId, elite, reagentTagId, aItemMap, loot);

						for (const Loot& t : loot)
						{
							Data::Item::Zone* oracleZone = aItemMap.GetItemOracle(t.m_itemId)->GetOrCreateZone(aMap->m_id, zone != NULL ? zone->m_id : 0);

							if(zone != NULL && zone->m_preposition.empty())
							{
								std::string error = Helpers::Format("[%s:%u] Zone must have preposition defined when used by oracle.", zone->m_debugInfo->m_file.c_str(), zone->m_debugInfo->m_line);
								aOutErrors.insert(error);
							}

							oracleZone->AddUniqueLootGroupId(t.m_lootGroupId);
						}
					}
				}
			}
		}
	}

	namespace PostProcessMaps
	{

		void		
		Run(
			Manifest*									aManifest)
		{
			ItemMap itemMap;
			itemMap.Build(aManifest);

			std::unordered_set<std::string> errors;

			aManifest->GetContainer<Data::Map>()->ForEach([&](
				Data::Map* aMap)
			{
				for(const MapData::EntitySpawn& entitySpawn : aMap->m_data->m_entitySpawns)
				{
					if(entitySpawn.m_entityId != 0)
					{
						_EntityAtPosition(aManifest, { entitySpawn.m_x, entitySpawn.m_y }, entitySpawn.m_entityId, aMap, itemMap, errors);
					}
					else
					{
						const Data::MapEntitySpawn* mapEntitySpawn = aManifest->GetById<Data::MapEntitySpawn>(entitySpawn.m_mapEntitySpawnId);
						for(const std::unique_ptr<Data::MapEntitySpawn::Entity>& entity : mapEntitySpawn ->m_entities)
							_EntityAtPosition(aManifest, { entitySpawn.m_x, entitySpawn.m_y }, entity->m_entityId, aMap, itemMap, errors);
					}
				}
				return true;
			});			

			for(const std::string& error : errors)
				printf("\x1B[31mERROR: %s\x1B[37m\n", error.c_str());

			TP_CHECK(errors.size() == 0, "%zu error%s encountered while processing maps.", errors.size(), errors.size() == 1 ? "" : "s");
		}

	}

}