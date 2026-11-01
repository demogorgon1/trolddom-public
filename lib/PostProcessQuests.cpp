#include "Pcheader.h"

#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/VendorStock.h>

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/DialogueRoot.h>
#include <tpublic/Data/DialogueScreen.h>
#include <tpublic/Data/Encounter.h>
#include <tpublic/Data/Entity.h>
#include <tpublic/Data/Item.h>
#include <tpublic/Data/LootTable.h>
#include <tpublic/Data/Map.h>
#include <tpublic/Data/MapEntitySpawn.h>
#include <tpublic/Data/Objective.h>
#include <tpublic/Data/Quest.h>

#include <tpublic/DirectEffects/ConsumeAndProduce.h>
#include <tpublic/DirectEffects/Simple.h>
#include <tpublic/DirectEffects/SpawnEntity.h>

#include <tpublic/ObjectiveTypes/CollectItems.h>

#include <tpublic/Manifest.h>
#include <tpublic/MapData.h>

namespace tpublic
{

	namespace
	{

		std::string
		_Location(
			const DataBase*							aData)
		{
			if(!aData->m_debugInfo.has_value())
				return "";
			return Helpers::Format("[%s:%u] ", aData->m_debugInfo->m_file.c_str(), aData->m_debugInfo->m_line);
		}

		void
		_AddMapEntitySpawn(
			const Manifest*							aManifest,
			uint32_t								aMapEntitySpawnId,
			std::unordered_set<uint32_t>&			aOut)
		{
			if(aMapEntitySpawnId == 0)
				return;

			const Data::MapEntitySpawn* mapEntitySpawn = aManifest->GetById<Data::MapEntitySpawn>(aMapEntitySpawnId);
			for(const std::unique_ptr<Data::MapEntitySpawn::Entity>& entity : mapEntitySpawn->m_entities)
				aOut.insert(entity->m_entityId);
		}

		void
		_AddDialogueScreen(
			const Manifest*							aManifest,
			uint32_t								aDialogueScreenId,
			std::unordered_set<uint32_t>&			aVisited,
			std::unordered_set<uint32_t>&			aOutOfferedQuestIds,
			std::unordered_set<uint32_t>&			aOutCompletedQuestIds)
		{
			if(aDialogueScreenId == 0 || aVisited.contains(aDialogueScreenId))
				return;

			aVisited.insert(aDialogueScreenId);

			const Data::DialogueScreen* dialogueScreen = aManifest->GetById<Data::DialogueScreen>(aDialogueScreenId);
			for(const Data::DialogueScreen::Option& option : dialogueScreen->m_options)
			{
				if(option.m_questId != 0)
				{
					if(option.m_questOffer)
						aOutOfferedQuestIds.insert(option.m_questId);
					if(option.m_questCompletion)
						aOutCompletedQuestIds.insert(option.m_questId);
				}

				_AddDialogueScreen(aManifest, option.m_dialogueScreenId, aVisited, aOutOfferedQuestIds, aOutCompletedQuestIds);
			}
		}

		// Finds quests that can't be done: quests nobody accepts the completion of, quests with prerequisites that can't be
		// done, objectives that can't progress and items to collect that can't be obtained (errors), and quests nothing offers
		// (warnings, they might just not be assigned yet).
		void
		_Validate(
			const Manifest*							aManifest)
		{
			std::vector<std::string> errors;

			// Entities that exist somewhere: placed on maps, or spawned by abilities, encounters and map generators
			std::unordered_map<uint32_t, std::unordered_set<uint32_t>> mapEntityIds;
			std::unordered_set<uint32_t> spawnedEntityIds;
			std::unordered_set<uint32_t> placedMapEntitySpawnIds;

			aManifest->GetContainer<Data::Map>()->ForEach([&](
				const Data::Map* aMap)
			{
				std::unordered_set<uint32_t>& entityIds = mapEntityIds[aMap->m_id];
				for(const MapData::EntitySpawn& entitySpawn : aMap->m_data->m_entitySpawns)
				{
					if(entitySpawn.m_entityId != 0)
						entityIds.insert(entitySpawn.m_entityId);
					_AddMapEntitySpawn(aManifest, entitySpawn.m_mapEntitySpawnId, entityIds);
					placedMapEntitySpawnIds.insert(entitySpawn.m_mapEntitySpawnId);
				}
				return true;
			});

			// Map entity spawns that aren't placed on a map are used some other way (for example by generated maps like the
			// gateway worlds)
			aManifest->GetContainer<Data::MapEntitySpawn>()->ForEach([&](
				const Data::MapEntitySpawn* aMapEntitySpawn)
			{
				if(!placedMapEntitySpawnIds.contains(aMapEntitySpawn->m_id))
					_AddMapEntitySpawn(aManifest, aMapEntitySpawn->m_id, spawnedEntityIds);
				return true;
			});

			std::unordered_set<uint32_t> startQuestIds;
			std::unordered_set<uint32_t> producedItemIds;

			aManifest->GetContainer<Data::Ability>()->ForEach([&](
				const Data::Ability* aAbility)
			{
				for(const std::unique_ptr<Data::Ability::DirectEffectEntry>& directEffect : aAbility->m_directEffects)
				{
					if(directEffect->m_directEffectId == DirectEffect::ID_SPAWN_ENTITY)
					{
						const DirectEffects::SpawnEntity* spawnEntity = (const DirectEffects::SpawnEntity*)directEffect->m_directEffectBase.get();
						for(uint32_t entityId : spawnEntity->m_entityIds)
							spawnedEntityIds.insert(entityId);
						_AddMapEntitySpawn(aManifest, spawnEntity->m_mapEntitySpawnId, spawnedEntityIds);
					}
					else if(directEffect->m_directEffectId == DirectEffect::ID_SIMPLE)
					{
						const DirectEffects::Simple* simple = (const DirectEffects::Simple*)directEffect->m_directEffectBase.get();
						if(simple->m_id == SimpleDirectEffect::ID_START_QUEST)
							startQuestIds.insert(simple->m_param);
					}
					else if(directEffect->m_directEffectId == DirectEffect::ID_CONSUME_AND_PRODUCE)
					{
						const DirectEffects::ConsumeAndProduce* consumeAndProduce = (const DirectEffects::ConsumeAndProduce*)directEffect->m_directEffectBase.get();
						producedItemIds.insert(consumeAndProduce->m_produceSourceItemId);
					}
				}

				for(const std::unique_ptr<Data::Ability::AOEEntitySpawnEntry>& aoeEntitySpawn : aAbility->m_aoeEntitySpawns)
					spawnedEntityIds.insert(aoeEntitySpawn->m_entityId);

				if(aAbility->m_produceItems)
				{
					for(const Data::Ability::Item& item : aAbility->m_produceItems->m_items)
						producedItemIds.insert(item.m_itemId);
				}
				return true;
			});

			aManifest->GetContainer<Data::Encounter>()->ForEach([&](
				const Data::Encounter* aEncounter)
			{
				for(const std::unique_ptr<Data::Encounter::Spawn>& spawn : aEncounter->m_spawns)
				{
					_AddMapEntitySpawn(aManifest, spawn->m_mapEntitySpawnId, spawnedEntityIds);
					for(uint32_t entityId : spawn->m_possibleEntityIds)
						spawnedEntityIds.insert(entityId);
				}
				for(uint32_t entityId : aEncounter->m_mainEntityIds)
					spawnedEntityIds.insert(entityId);
				return true;
			});

			std::unordered_set<uint32_t> existingEntityIds = spawnedEntityIds;
			for(const auto& [mapId, entityIds] : mapEntityIds)
				existingEntityIds.insert(entityIds.begin(), entityIds.end());

			// Quests offered and completed by entities that exist, and items sold by them
			std::unordered_set<uint32_t> offeredQuestIds = startQuestIds;
			std::unordered_set<uint32_t> completedQuestIds;
			std::unordered_set<uint32_t> visitedDialogueScreenIds;

			aManifest->GetContainer<Data::Entity>()->ForEach([&](
				const Data::Entity* aEntity)
			{
				if(!existingEntityIds.contains(aEntity->m_id))
					return true;

				const Components::CombatPublic* combatPublic = aEntity->TryGetComponent<Components::CombatPublic>();
				if(combatPublic != NULL && combatPublic->m_dialogueRootId != 0)
				{
					const Data::DialogueRoot* dialogueRoot = aManifest->GetById<Data::DialogueRoot>(combatPublic->m_dialogueRootId);
					for(const std::unique_ptr<Data::DialogueRoot::Entry>& entry : dialogueRoot->m_entries)
						_AddDialogueScreen(aManifest, entry->m_dialogueScreenId, visitedDialogueScreenIds, offeredQuestIds, completedQuestIds);
				}

				const Components::VendorStock* vendorStock = aEntity->TryGetComponent<Components::VendorStock>();
				if(vendorStock != NULL)
				{
					for(const Components::VendorStock::Item& item : vendorStock->m_itemList)
						producedItemIds.insert(item.m_itemId);
				}
				return true;
			});

			for(uint32_t dialogueScreenId : visitedDialogueScreenIds)
			{
				const Data::DialogueScreen* dialogueScreen = aManifest->GetById<Data::DialogueScreen>(dialogueScreenId);
				for(const Data::DialogueScreen::Sell& sell : dialogueScreen->m_sell)
					producedItemIds.insert(sell.m_itemId);
			}

			// Items from loot tables
			std::unordered_set<uint32_t> usedLootGroupIds;
			aManifest->GetContainer<Data::LootTable>()->ForEach([&](
				const Data::LootTable* aLootTable)
			{
				for(const std::unique_ptr<Data::LootTable::Slot>& slot : aLootTable->m_slots)
				{
					for(const Data::LootTable::Possibility& possibility : slot->m_possibilities)
						usedLootGroupIds.insert(possibility.m_lootGroupId);
				}
				return true;
			});

			aManifest->GetContainer<Data::Item>()->ForEach([&](
				const Data::Item* aItem)
			{
				if(aItem->m_questId != 0)
					offeredQuestIds.insert(aItem->m_questId);

				for(uint32_t lootGroupId : aItem->m_lootGroups)
				{
					if(usedLootGroupIds.contains(lootGroupId))
						producedItemIds.insert(aItem->m_id);
				}
				return true;
			});

			aManifest->GetContainer<Data::Quest>()->ForEach([&](
				const Data::Quest* aQuest)
			{
				if(aQuest->m_nextQuestId != 0)
					offeredQuestIds.insert(aQuest->m_nextQuestId);

				for(uint32_t itemId : aQuest->m_questItems)
					producedItemIds.insert(itemId);
				for(uint32_t itemId : aQuest->m_rewardOneItem)
					producedItemIds.insert(itemId);
				for(uint32_t itemId : aQuest->m_rewardAllItems)
					producedItemIds.insert(itemId);
				return true;
			});

			// Check objectives. Problems are reported with the quests using them.
			std::unordered_map<uint32_t, std::vector<std::string>> objectiveProblems;

			aManifest->GetContainer<Data::Objective>()->ForEach([&](
				const Data::Objective* aObjective)
			{
				std::vector<std::string>& problems = objectiveProblems[aObjective->m_id];

				std::vector<uint32_t> watchedEntityIds;
				aObjective->m_objectiveTypeBase->GetWatchedEntities(watchedEntityIds);

				// Kill player objectives watch entity 0 (players)
				bool watchesPlayers = std::find(watchedEntityIds.begin(), watchedEntityIds.end(), 0u) != watchedEntityIds.end();

				if(watchedEntityIds.size() > 0 && aObjective->m_mapIds.empty())
				{
					problems.push_back(Helpers::Format("%sObjective '%s' doesn't list any maps, so it can't progress anywhere.", _Location(aObjective).c_str(), aObjective->m_name.c_str()));
				}
				else if(watchedEntityIds.size() > 0 && !watchesPlayers)
				{
					bool found = false;
					for(uint32_t entityId : watchedEntityIds)
					{
						if(spawnedEntityIds.contains(entityId))
							found = true;

						for(uint32_t mapId : aObjective->m_mapIds)
						{
							if(mapEntityIds[mapId].contains(entityId))
								found = true;
						}
					}

					if(!found)
						problems.push_back(Helpers::Format("%sObjective '%s': none of its entities are placed on its maps or spawned by anything.", _Location(aObjective).c_str(), aObjective->m_name.c_str()));
				}

				if(aObjective->m_objectiveTypeId == ObjectiveType::ID_COLLECT_ITEMS)
				{
					const ObjectiveTypes::CollectItems* collectItems = (const ObjectiveTypes::CollectItems*)aObjective->m_objectiveTypeBase.get();
					if(!producedItemIds.contains(collectItems->GetItemId()))
					{
						const Data::Item* item = aManifest->GetById<Data::Item>(collectItems->GetItemId());
						problems.push_back(Helpers::Format("%sObjective '%s': item '%s' can't be obtained (no loot table, vendor, quest or ability gives it).", _Location(aObjective).c_str(), aObjective->m_name.c_str(), item->m_name.c_str()));
					}
				}
				return true;
			});

			// Check quests. Quests that nothing offers are only warnings (they might not be assigned yet), but quests that are
			// offered must work.
			std::vector<std::string> warnings;
			std::unordered_set<uint32_t> reportedObjectiveIds;

			aManifest->GetContainer<Data::Quest>()->ForEach([&](
				const Data::Quest* aQuest)
			{
				if(!offeredQuestIds.contains(aQuest->m_id))
				{
					warnings.push_back(Helpers::Format("%sQuest '%s' isn't offered by anything (no NPC or object that exists, item, ability or previous quest).", _Location(aQuest).c_str(), aQuest->m_name.c_str()));
					return true;
				}

				if(!completedQuestIds.contains(aQuest->m_id))
					errors.push_back(Helpers::Format("%sQuest '%s' can't be turned in (no NPC or object that exists completes it).", _Location(aQuest).c_str(), aQuest->m_name.c_str()));

				for(uint32_t objectiveId : aQuest->m_objectives)
				{
					if(reportedObjectiveIds.contains(objectiveId))
						continue;

					reportedObjectiveIds.insert(objectiveId);

					for(const std::string& problem : objectiveProblems[objectiveId])
						errors.push_back(problem);
				}

				for(uint32_t prerequisiteId : aQuest->m_prerequisites)
				{
					if(!offeredQuestIds.contains(prerequisiteId))
						errors.push_back(Helpers::Format("%sQuest '%s' requires quest '%s', which isn't offered by anything.", _Location(aQuest).c_str(), aQuest->m_name.c_str(), aManifest->GetById<Data::Quest>(prerequisiteId)->m_name.c_str()));
				}

				// Prerequisite loop
				{
					std::unordered_set<uint32_t> visited;
					std::vector<uint32_t> stack = aQuest->m_prerequisites;
					while(stack.size() > 0)
					{
						uint32_t questId = stack.back();
						stack.pop_back();

						if(questId == aQuest->m_id)
						{
							errors.push_back(Helpers::Format("%sQuest '%s' requires itself (prerequisite loop).", _Location(aQuest).c_str(), aQuest->m_name.c_str()));
							break;
						}

						if(visited.contains(questId))
							continue;

						visited.insert(questId);

						const Data::Quest* prerequisite = aManifest->GetById<Data::Quest>(questId);
						stack.insert(stack.end(), prerequisite->m_prerequisites.begin(), prerequisite->m_prerequisites.end());
					}
				}
				return true;
			});

			std::sort(warnings.begin(), warnings.end());
			for(const std::string& warning : warnings)
				printf("\x1B[33mWARNING: %s\x1B[37m\n", warning.c_str());

			std::sort(errors.begin(), errors.end());
			for(const std::string& error : errors)
				printf("\x1B[31mERROR: %s\x1B[37m\n", error.c_str());

			TP_CHECK(errors.size() == 0, "%zu error%s encountered while checking quests.", errors.size(), errors.size() == 1 ? "" : "s");
		}

	}

	namespace PostProcessQuests
	{

		void		
		Run(
			Manifest* aManifest)
		{
			// Figure out which items should be flagged as quest rewards
			{
				std::unordered_set<uint32_t> questRewardItemIds;

				aManifest->GetContainer<Data::Quest>()->ForEach([&questRewardItemIds](
					Data::Quest* aQuest)
				{
					for (uint32_t itemId : aQuest->m_rewardAllItems)
						questRewardItemIds.insert(itemId);
					for (uint32_t itemId : aQuest->m_rewardOneItem)
						questRewardItemIds.insert(itemId);
					return true;
				});

				aManifest->GetContainer<Data::Item>()->ForEach([&questRewardItemIds](
					Data::Item* aItem)
				{
					if (questRewardItemIds.contains(aItem->m_id))
						aItem->m_flags |= Data::Item::FLAG_QUEST_REWARD;
					return true;
				});
			}

			_Validate(aManifest);
		}

	}

}