#include "Pcheader.h"

#include <tpublic/Components/Lootable.h>

#include <tpublic/Data/CreatureType.h>
#include <tpublic/Data/Item.h>
#include <tpublic/Data/LootGroup.h>
#include <tpublic/Data/LootTable.h>

#include <tpublic/LootGenerator.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	LootGenerator::LootGenerator(
		const Manifest*			aManifest)
		: m_manifest(aManifest)
	{
		m_manifest->GetContainer<tpublic::Data::Item>()->ForEach([&](
			const Data::Item*	aItem)
		{
			UIntRange levelRange = aItem->m_levelRange;

			if (levelRange.m_min == 0)
			{
				levelRange.m_max = aItem->m_requiredLevel + 1;
				levelRange.m_min = aItem->m_requiredLevel - 1;

				if(levelRange.m_min < 1)
					levelRange.m_min = 1;
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
		});
	}

	LootGenerator::~LootGenerator()
	{

	}

	void		
	LootGenerator::GenerateLootable(
		std::mt19937&			aRandom,
		uint32_t				aLevel,
		uint32_t				aCreatureTypeId,
		bool					aIsElite,
		Components::Lootable*	aLootable) const
	{
		const Data::LootTable* lootTable = m_manifest->GetById<tpublic::Data::LootTable>(aLootable->m_lootTableId);

		if(lootTable->m_cash.has_value())
		{
			std::uniform_int_distribution<int64_t> distribution(lootTable->m_cash->m_min, lootTable->m_cash->m_max);
			aLootable->m_availableCash = distribution(aRandom);
			aLootable->m_cash = aLootable->m_availableCash > 0;
		}
		else if(aCreatureTypeId != 0)
		{
			if(m_manifest->GetById<tpublic::Data::CreatureType>(aCreatureTypeId)->m_flags & Data::CreatureType::FLAG_CASH_LOOT)
			{
				const NPCMetrics::Level* npcMetricsLevel = m_manifest->m_npcMetrics.GetLevel(aLevel);
				if (npcMetricsLevel != NULL)
				{
					std::uniform_int_distribution<uint32_t> distribution(npcMetricsLevel->m_cash.m_min, npcMetricsLevel->m_cash.m_max);
					aLootable->m_availableCash = (int64_t)distribution(aRandom);

					if(aLootable->m_availableCash > 0 && aIsElite)
					{
						aLootable->m_availableCash = (int64_t)((float)aLootable->m_availableCash * npcMetricsLevel->m_eliteCash);
					}

					aLootable->m_cash = aLootable->m_availableCash > 0;						
				}
			}
		}

		for(const std::unique_ptr<Data::LootTable::Slot>& slot : lootTable->m_slots)
		{
			uint32_t totalWeight = 0;

			for(const Data::LootTable::Possibility& possibility : slot->m_possibilities)
				totalWeight += possibility.m_weight;

			std::uniform_int_distribution<uint32_t> distribution(1, totalWeight);
			uint32_t possibilityRoll = distribution(aRandom);
			uint32_t accumWeight = 0;
			uint32_t lootGroupId = 0;

			for (const Data::LootTable::Possibility& possibility : slot->m_possibilities)
			{
				accumWeight += possibility.m_weight;

				if(possibilityRoll <= accumWeight)
				{
					lootGroupId = possibility.m_lootGroupId;
					break;
				}
			}

			if(lootGroupId != 0)
			{
				GroupTable::const_iterator i = m_groups.find(lootGroupId);
				if(i != m_groups.end())
				{
					const Group* group = i->second.get();
					const LevelBucket* levelBucket = group->GetLevelBucket(aLevel);

					uint32_t itemId = 0;

					if(levelBucket != NULL)
					{
						size_t totalItemCount = group->m_defaultLevelBucket.m_itemIds.size() + levelBucket->m_itemIds.size();
						assert(totalItemCount > 0);
						std::uniform_int_distribution<size_t> d(0, totalItemCount - 1);
						size_t roll = d(aRandom);
						if(roll < group->m_defaultLevelBucket.m_itemIds.size())
							itemId = group->m_defaultLevelBucket.m_itemIds[roll];
						else
							itemId = levelBucket->m_itemIds[roll - group->m_defaultLevelBucket.m_itemIds.size()];
					}
					else if(group->m_defaultLevelBucket.m_itemIds.size() > 0)
					{
						std::uniform_int_distribution<size_t> d(0, group->m_defaultLevelBucket.m_itemIds.size() - 1);
						size_t roll = d(aRandom);
						itemId = group->m_defaultLevelBucket.m_itemIds[roll];
					}

					if(itemId != 0)
					{
						Components::Lootable::AvailableLoot loot;
						loot.m_itemInstance.m_itemId = itemId;
						loot.m_itemInstance.m_seed = aRandom();
						aLootable->m_availableLoot.push_back(loot);
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