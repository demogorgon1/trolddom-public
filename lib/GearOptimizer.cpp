#include "Pcheader.h"

#include <tpublic/GearOptimizer.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	GearOptimizer::GearOptimizer(
		const Manifest*							aManifest)
		: m_manifest(aManifest)
	{

	}
	
	GearOptimizer::~GearOptimizer()
	{

	}

	void	
	GearOptimizer::MakeEquippedItems(
		Rarity::Id								aMaxRarity,
		uint32_t								aMaxLevel,
		const Data::Class*						aClass,
		const char*								aGearOptimizationName,
		Components::EquippedItems::Slots&		aOut)
	{
		const Data::Class::GearOptimization* gearOptimization = aClass->GetGearOptimization(aGearOptimizationName);
		TP_CHECK(gearOptimization != NULL, "'%s' is not a valid gear optimization name.", aGearOptimizationName);
		
		for(uint32_t i = 1; i < (uint32_t)EquipmentSlot::NUM_IDS; i++)
		{
			EquipmentSlot::Id equipmentSlotId = (EquipmentSlot::Id)i;
			if(equipmentSlotId == EquipmentSlot::ID_OFF_HAND)
				continue; // Ignore off-hand for now

			float bestScore = 0.0f;
			const Data::Item* bestItem = NULL;

			m_manifest->GetContainer<Data::Item>()->ForEach([&](
				const Data::Item* aItem)
			{
				if(aItem->m_requiredLevel <= aMaxLevel && aItem->m_rarity <= aMaxRarity && (aItem->m_itemType == ItemType::ID_MISCELLANEOUS || aClass->CanUseItemType(aItem->m_itemType)) && aItem->IsEquippableInSlot(i))
				{
					float score = _ScoreItem(aItem, gearOptimization);
					if(bestItem == NULL || score > bestScore)
					{
						bestItem = aItem;
						bestScore = score;
					}
				}

				return true;
			});

			if(bestItem != NULL)
			{
				ItemInstance itemInstance;
				itemInstance.m_itemId = bestItem->m_id;				
				
				if(bestItem->m_itemBinding != ItemBinding::ID_NEVER)
					itemInstance.SetSoulbound();

				aOut.m_items[i] = itemInstance;
			}
		}	
	}

	//-----------------------------------------------------------------------------------------

	float	
	GearOptimizer::_ScoreItem(
		const Data::Item*						aItem,
		const Data::Class::GearOptimization*	aGearOptimization) const
	{
		ItemInstance itemInstance(aItem->m_id);
		ItemInstanceData itemInstanceData(m_manifest, itemInstance);

		float score = 0.0f;

		if (itemInstanceData.m_isWeapon)
			score += (10.0f * (float)((itemInstanceData.m_weaponDamage.m_max + itemInstanceData.m_weaponDamage.m_min) / 2) / (float)itemInstanceData.m_weaponCooldown) * aGearOptimization->m_weaponDPSWeight;

		for(const Data::Class::GearOptimization::Entry& t : aGearOptimization->m_entries)
			score += t.m_weight * itemInstanceData.m_stats.m_stats[t.m_statId];

		return score;
	}

}