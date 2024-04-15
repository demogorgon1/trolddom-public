#include "Pcheader.h"

#include <tpublic/Helpers.h>
#include <tpublic/ItemInstanceData.h>
#include <tpublic/ItemType.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	ItemInstanceData::ItemInstanceData()
	{
	
	}

	ItemInstanceData::ItemInstanceData(
		const Manifest*		aManifest,
		const ItemInstance&	aItemInstance)
		: m_instance(aItemInstance)
	{
		m_itemData = aManifest->GetById<tpublic::Data::Item>(aItemInstance.m_itemId);

		const ItemType::Info* itemTypeInfo = ItemType::GetInfo(m_itemData->m_itemType);
		const ItemMetrics::Multipliers& rarityMultipliers = aManifest->m_itemMetrics.GetRarityMultipliers(m_itemData->m_rarity);
		const ItemMetrics::Multipliers& itemTypeMultipliers = aManifest->m_itemMetrics.GetItemTypeMultipliers(m_itemData->m_itemType);
		const ItemMetrics::Multipliers& equipmentSlotMultipliers = aManifest->m_itemMetrics.GetEquipmentSlotMultipliers(m_itemData->m_equipmentSlots);

		ItemMetrics::Multipliers multipliers;
		multipliers.m_armor = itemTypeMultipliers.m_armor * equipmentSlotMultipliers.m_armor * rarityMultipliers.m_armor;
		multipliers.m_cost = itemTypeMultipliers.m_cost * equipmentSlotMultipliers.m_cost * rarityMultipliers.m_cost;
		multipliers.m_weaponDamage = itemTypeMultipliers.m_weaponDamage * equipmentSlotMultipliers.m_weaponDamage * rarityMultipliers.m_weaponDamage;

		{
			Stat::Collection statWeights;
			uint32_t totalStatWeight = 0;

			m_statBudget = (int32_t)m_itemData->m_itemLevel - 1;			
			m_statBudget = (int32_t)(rarityMultipliers.m_statBudget * (float)m_statBudget);

			_Generate(aManifest, statWeights, totalStatWeight);

			if(totalStatWeight > 0 && m_statBudget > 0)
			{
				for(uint32_t i = 1; i < (uint32_t)Stat::NUM_IDS; i++)
				{
					const Stat::Info* statInfo = Stat::GetInfo((Stat::Id)i);					
					m_stats.m_stats[i] += (statWeights.m_stats[i] * (float)m_statBudget) / ((float)totalStatWeight * statInfo->m_budgetCost);
				}
			}
		}

		if(itemTypeInfo->m_flags & ItemType::FLAG_WEAPON)
		{
			if(m_weaponCooldown == 0)
				m_weaponCooldown = 20;

			if (m_weaponDamage.m_min == 0 && m_weaponDamage.m_max == 0)
			{
				// This weapon doesn't have explicit damage range, use defaults based on item level
				bool isTwoHanded = itemTypeInfo->m_flags & ItemType::FLAG_TWO_HANDED;
				uint32_t dps = isTwoHanded ? aManifest->m_itemMetrics.GetLevelBase2HWeaponDPS(m_itemData->m_itemLevel) : aManifest->m_itemMetrics.GetLevelBase1HWeaponDPS(m_itemData->m_itemLevel);

				dps = (uint32_t)((float)dps * multipliers.m_weaponDamage);

				uint32_t avgDamage = (dps * m_weaponCooldown) / 10;
				
				m_weaponDamage.m_min = avgDamage / 2;
				m_weaponDamage.m_max = (avgDamage * 3) / 2;
				
				if(m_weaponDamage.m_min == 0)
					m_weaponDamage.m_min = 1;

				if(m_weaponDamage.m_max <= m_weaponDamage.m_min)
					m_weaponDamage.m_max = m_weaponDamage.m_min + 1;
			}

			m_isWeapon = true;
		}

		if(itemTypeInfo->m_flags & ItemType::FLAG_ARMOR)
		{
			uint32_t baseArmor = aManifest->m_itemMetrics.GetLevelBaseArmor(m_itemData->m_itemLevel);
			baseArmor = 1 + (uint32_t)((float)baseArmor * multipliers.m_armor);
			m_stats.m_stats[Stat::ID_ARMOR] += (float)baseArmor; 

			if (itemTypeInfo->m_flags & ItemType::FLAG_SHIELD)
			{
				uint32_t blockValue = (uint32_t)((float)baseArmor * aManifest->m_itemMetrics.m_shieldArmorToBaseBlockValue);
				if(blockValue == 0)
					blockValue = 1;
				m_stats.m_stats[Stat::ID_BLOCK_VALUE] += (float)blockValue;
			}
		}

		if (m_cost == 0)
		{
			uint32_t baseCost = aManifest->m_itemMetrics.GetLevelBaseCost(m_itemData->m_itemLevel);
			baseCost = (uint32_t)((float)baseCost * multipliers.m_cost * rarityMultipliers.m_cost);
			m_cost = baseCost;
		}

		m_vendorValue = (uint32_t)((float)m_cost * aManifest->m_itemMetrics.m_vendorCostMultiplier);
	}
	
	ItemInstanceData::~ItemInstanceData()
	{

	}

	//---------------------------------------------------------

	void		
	ItemInstanceData::_Generate(
		const Manifest*			/*aManifest*/,
		Stat::Collection&		aOutStatWeights,
		uint32_t&				aOutTotalStatWeight)
	{
		if(!m_itemData->m_string.empty())
			m_name = m_itemData->m_string;

		if (!m_itemData->m_flavor.empty())
			m_flavor = m_itemData->m_flavor;

		m_statBudget += m_itemData->m_budgetBias;

		if(m_itemData->m_weaponDamage.has_value())
			m_weaponDamage = m_itemData->m_weaponDamage.value();

		if(m_itemData->m_weaponCooldown != 0)
			m_weaponCooldown = m_itemData->m_weaponCooldown;

		for(const Data::Item::AddedStat& addedStat : m_itemData->m_addedStats)
		{
			if(!addedStat.m_isBudgetWeight)
			{
				m_stats.m_stats[addedStat.m_id] += (float)addedStat.m_value;
			}
			else
			{				
				uint32_t weight = addedStat.m_value;
				aOutStatWeights.m_stats[addedStat.m_id] += (float)weight;
				aOutTotalStatWeight += weight;
			}
		}

		
	}

}