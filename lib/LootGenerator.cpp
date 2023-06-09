#include "Pcheader.h"

#include <tpublic/Components/Lootable.h>

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
		m_manifest->m_items.ForEach([&](
			const Data::Item*	aItem)
		{
			for(uint32_t lootGroupId : aItem->m_lootGroups)
				_GetOrCreateGroup(lootGroupId)->m_itemIds.push_back(aItem->m_id);
		});
	}

	LootGenerator::~LootGenerator()
	{

	}

	void		
	LootGenerator::Generate(
		std::mt19937&			aRandom,
		Components::Lootable*	aLootable) const
	{
		const Data::LootTable* lootTable = m_manifest->m_lootTables.GetById(aLootable->m_lootTableId);

		if(lootTable->m_cash.has_value())
		{
			std::uniform_int_distribution<int64_t> distribution(lootTable->m_cash->m_min, lootTable->m_cash->m_max);
			aLootable->m_availableCash = distribution(aRandom);
		}

		for(const std::unique_ptr<Data::LootTable::Slot>& slot : lootTable->m_slots)
		{
			uint32_t totalWeight = 0;

			for(const Data::LootTable::Possibility& possibility : slot->m_possibilities)
				totalWeight += possibility.m_weight;

			std::uniform_int_distribution<uint32_t> distribution(0, totalWeight);
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
					ItemInstance itemInstance;
					itemInstance.m_itemId = i->second->GetRandom(aRandom);
					itemInstance.m_seed = aRandom();
					aLootable->m_availableLoot.push_back(itemInstance);
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