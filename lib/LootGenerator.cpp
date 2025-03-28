#include "Pcheader.h"

#include <tpublic/Components/ActiveQuests.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/KillContribution.h>
#include <tpublic/Components/Lootable.h>
#include <tpublic/Components/PlayerPublic.h>

#include <tpublic/Data/CreatureType.h>
#include <tpublic/Data/Item.h>
#include <tpublic/Data/LootGroup.h>
#include <tpublic/Data/LootTable.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/LootGenerator.h>
#include <tpublic/Manifest.h>
#include <tpublic/Requirements.h>

namespace tpublic
{

	LootGenerator::LootGenerator(
		const Manifest*			aManifest)
		: m_manifest(aManifest)
	{
		m_manifest->GetContainer<Data::Item>()->ForEach([&](
			const Data::Item*	aItem)
		{
			UIntRange levelRange = aItem->m_levelRange;

			if (levelRange.m_min == 0 && aItem->m_itemLevel > 0)
			{
				levelRange.m_max = aItem->m_itemLevel + 2;
				levelRange.m_min = aItem->m_itemLevel;

				if(levelRange.m_min < 1)
					levelRange.m_min = 1;

				// For rare and epic items, make it possible for higher level NPCs to drop them as well
				if (aItem->m_rarity == Rarity::ID_RARE || aItem->m_rarity == Rarity::ID_EPIC)
					levelRange.m_max += 2;
			}

			for(uint32_t lootGroupId : aItem->m_lootGroups)
			{
				Group* group = _GetOrCreateGroup(lootGroupId);
				
				if(levelRange.m_min == 0)
				{
					group->m_defaultLevelBucket.m_itemIds.push_back(aItem->m_id);
				}
				else
				{
					for(uint32_t level = levelRange.m_min; level <= levelRange.m_max; level++)
						group->GetOrCreateLevelBucket(level)->m_itemIds.push_back(aItem->m_id);
				}				
			}
			return true;
		});
	}

	LootGenerator::~LootGenerator()
	{

	}

	void		
	LootGenerator::GenerateLootable(
		std::mt19937&								aRandom,
		const std::vector<const EntityInstance*>&	aPlayerEntityInstances,
		const EntityInstance*						aLootableEntityInstance,
		uint32_t									aLevel,
		uint32_t									aCreatureTypeId,
		bool										aIsElite,
		uint32_t									aPlayerWorldCharacterId,
		Components::Lootable*						aLootable) const
	{		
		if(aLootable->m_lootTableId == 0)
			return;

		const Data::LootTable* lootTable = m_manifest->GetById<tpublic::Data::LootTable>(aLootable->m_lootTableId);

		// Cash
		if(aPlayerWorldCharacterId == 0)
		{
			if(lootTable->m_cash.has_value())
			{
				tpublic::UniformDistribution<int64_t> distribution(lootTable->m_cash->m_min, lootTable->m_cash->m_max);
				aLootable->m_availableCash = distribution(aRandom);
				aLootable->m_cash = aLootable->m_availableCash > 0;
			}
			else if(aCreatureTypeId == 0 || m_manifest->GetById<tpublic::Data::CreatureType>(aCreatureTypeId)->m_flags & Data::CreatureType::FLAG_CASH_LOOT)
			{
				const NPCMetrics::Level* npcMetricsLevel = m_manifest->m_npcMetrics.GetLevel(aLevel);
				if (npcMetricsLevel != NULL)
				{
					tpublic::UniformDistribution<uint32_t> distribution(npcMetricsLevel->m_cash.m_min, npcMetricsLevel->m_cash.m_max);
					aLootable->m_availableCash = (int64_t)distribution(aRandom);

					if(aLootable->m_availableCash > 0 && aIsElite)
						aLootable->m_availableCash = (int64_t)((float)aLootable->m_availableCash * npcMetricsLevel->m_eliteCash);

					aLootable->m_cash = aLootable->m_availableCash > 0;						
				}
			}

			if(aLootable->m_availableCash > 0)
				aLootable->m_availableCash = (int64_t)((float)aLootable->m_availableCash * lootTable->m_cashMultiplier);
		}

		// Items
		GenerateLootableItems(aRandom, aPlayerEntityInstances, aLootableEntityInstance, aLevel, aCreatureTypeId, lootTable, aPlayerWorldCharacterId, aLootable);
	}

	void		
	LootGenerator::GenerateLootableItems(
		std::mt19937&								aRandom,
		const std::vector<const EntityInstance*>&	aPlayerEntityInstances,
		const EntityInstance*						aLootableEntityInstance,
		uint32_t									aLevel,
		uint32_t									aCreatureTypeId,
		const Data::LootTable*						aLootTable,
		uint32_t									aPlayerWorldCharacterId,
		Components::Lootable*						aLootable) const
	{
		const Components::KillContribution* killContribution = aLootableEntityInstance->GetComponent<Components::KillContribution>();

		GenerateItems(aRandom, aPlayerEntityInstances, aLootableEntityInstance, aLevel, aCreatureTypeId, aLootTable, [&](
			const ItemInstance& aItemInstance,
			uint32_t			aLootCooldownId)			
		{			
			const Data::Item* item = m_manifest->GetById<Data::Item>(aItemInstance.m_itemId);

			Components::Lootable::AvailableLoot loot;
			loot.m_itemInstance = aItemInstance;
			loot.m_itemInstance.SetWorldbound(aPlayerWorldCharacterId);

			if(loot.m_itemInstance.m_quantity > 1)
			{
				if(loot.m_itemInstance.m_quantity > item->m_stackSize)
					loot.m_itemInstance.m_quantity = item->m_stackSize;

				if(item->IsUnique())
					loot.m_itemInstance.m_quantity = 1;
			}

			if(aLootCooldownId != 0)
			{
				// Items with loot cooldown goes to all players. Note that this doesn't work with quest items.
				for (const EntityInstance* playerEntityInstance : aPlayerEntityInstances)
				{
					const Components::PlayerPublic* playerPublic = playerEntityInstance->GetComponent<Components::PlayerPublic>();
					const Components::CombatPublic* combatPublic = playerEntityInstance->GetComponent<Components::CombatPublic>();

					loot.m_playerTag.SetCharacter(playerPublic->m_characterId, combatPublic->m_level);
					loot.m_lootCooldownId = aLootCooldownId;
					aLootable->m_availableLoot.push_back(loot);
				}
			}
			else if(item->m_questId != 0)
			{
				// This is a quest item. Generate a copy for everyone with the quest.
				for(const EntityInstance* playerEntityInstance : aPlayerEntityInstances)
				{
					const Components::ActiveQuests* activeQuests = playerEntityInstance->GetComponent<Components::ActiveQuests>();
					if(activeQuests->HasQuest(item->m_questId))
					{
						const Components::PlayerPublic* playerPublic = playerEntityInstance->GetComponent<Components::PlayerPublic>();
						const Components::CombatPublic* combatPublic = playerEntityInstance->GetComponent<Components::CombatPublic>();

						loot.m_playerTag.SetCharacter(playerPublic->m_characterId, combatPublic->m_level);
						loot.m_questId = item->m_questId;
						aLootable->m_availableLoot.push_back(loot);
					}
				}
			}
			else if(killContribution != NULL && item->IsKillContributionLoot())
			{
				// This can be looted by anyone with kill contribution. Generate a copy for everyone on the list.
				for(uint32_t characterId : killContribution->m_characterIds.m_ids)
				{
					loot.m_playerTag.SetCharacter(characterId, 0); // Player tag character level isn't relevant here
					aLootable->m_availableLoot.push_back(loot);
				}
			}
			else
			{
				aLootable->m_availableLoot.push_back(loot);
			}
		});
	}

	void		
	LootGenerator::GenerateItems(
		std::mt19937&								aRandom,
		const std::vector<const EntityInstance*>&	aPlayerEntityInstances,
		const EntityInstance*						aLootableEntityInstance,
		uint32_t									aLevel,
		uint32_t									aCreatureTypeId,
		const Data::LootTable*						aLootTable,
		ItemCallback								aItemCallback) const
	{	
		for(const std::unique_ptr<Data::LootTable::Slot>& slot : aLootTable->m_slots)
		{
			struct Entry
			{
				uint32_t	m_accumWeight = 0;
				uint32_t	m_lootGroupId = 0;
				uint32_t	m_quantity = 0;
				uint32_t	m_lootCooldownId = 0;
			};

			Entry entries[Data::LootTable::Slot::MAX_POSSIBILTY_COUNT];
			size_t entryCount = 0;

			uint32_t accumWeight = 0;

			for(const Data::LootTable::Possibility& possibility : slot->m_possibilities)
			{
				if(!possibility.HasCreatureType(aCreatureTypeId))
					continue;

				if(aLootableEntityInstance != NULL)
				{
					if (!Requirements::CheckAnyList(m_manifest, possibility.m_requirements, aPlayerEntityInstances, aLootableEntityInstance))
						continue;
				}

				TP_CHECK(entryCount < Data::LootTable::Slot::MAX_POSSIBILTY_COUNT, "Too many loot slot possibilities.");

				accumWeight += possibility.m_weight;
				Entry& t = entries[entryCount++];
				t.m_accumWeight = accumWeight;
				t.m_quantity = possibility.m_quantity;
				t.m_lootGroupId = possibility.m_lootGroupId;

				if(possibility.m_useSpecialLootCooldown && aLootableEntityInstance != NULL)
				{
					const Components::Lootable* lootable = aLootableEntityInstance->GetComponent<Components::Lootable>();
					if(lootable != NULL)
						t.m_lootCooldownId = lootable->m_specialLootCooldownId;
				}
				else
				{
					t.m_lootCooldownId = possibility.m_lootCooldownId;
				}
			}

			if(entryCount > 0)
			{
				tpublic::UniformDistribution<uint32_t> distribution(1, accumWeight);
				uint32_t possibilityRoll = distribution(aRandom);
				uint32_t lootGroupId = 0;
				uint32_t quantity = 0;
				uint32_t lootCooldownId = 0;

				for (size_t i = 0; i < entryCount; i++)
				{
					const Entry& t = entries[i];
					if (possibilityRoll <= t.m_accumWeight)
					{
						lootGroupId = t.m_lootGroupId;
						quantity = t.m_quantity;
						lootCooldownId = t.m_lootCooldownId;
						break;
					}
				}

				if (lootGroupId != 0)
				{
					GroupTable::const_iterator i = m_groups.find(lootGroupId);
					if (i != m_groups.end())
					{
						const Group* group = i->second.get();
						const LevelBucket* levelBucket = aLevel != 0 ? group->GetLevelBucket(aLevel) : NULL;

						uint32_t itemId = 0;

						if (levelBucket != NULL)
						{
							size_t totalItemCount = group->m_defaultLevelBucket.m_itemIds.size() + levelBucket->m_itemIds.size();
							assert(totalItemCount > 0);
							tpublic::UniformDistribution<size_t> d(0, totalItemCount - 1);
							size_t roll = d(aRandom);
							if (roll < group->m_defaultLevelBucket.m_itemIds.size())
								itemId = group->m_defaultLevelBucket.m_itemIds[roll];
							else
								itemId = levelBucket->m_itemIds[roll - group->m_defaultLevelBucket.m_itemIds.size()];
						}
						else if (group->m_defaultLevelBucket.m_itemIds.size() > 0)
						{
							tpublic::UniformDistribution<size_t> d(0, group->m_defaultLevelBucket.m_itemIds.size() - 1);
							size_t roll = d(aRandom);
							itemId = group->m_defaultLevelBucket.m_itemIds[roll];
						}

						if (itemId != 0)
						{
							tpublic::ItemInstance itemInstance;
							itemInstance.m_itemId = itemId;
							itemInstance.m_quantity = quantity;
							aItemCallback(itemInstance, lootCooldownId);
						}
					}
				}
			}
		}
	}

	//---------------------------------------------------------------------------------

	LootGenerator::Group* 
	LootGenerator::_GetOrCreateGroup(
		uint32_t				aLootGroupId)
	{
		GroupTable::iterator i = m_groups.find(aLootGroupId);
		if(i != m_groups.end())
			return i->second.get();

		Group* t = new Group();
		m_groups[aLootGroupId] = std::unique_ptr<Group>(t);
		return t;
	}

}