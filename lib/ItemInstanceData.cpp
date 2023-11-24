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
		m_itemData = aManifest->m_items.GetById(aItemInstance.m_itemId);
		std::mt19937 random(aItemInstance.m_seed);

		if(m_itemData->m_root)
			_Generate(random, aManifest, m_itemData->m_root.get());

		const ItemType::Info* itemTypeInfo = ItemType::GetInfo(m_itemData->m_itemType);
		uint32_t itemLevel = GetItemLevel();

		if(itemTypeInfo->m_flags & ItemType::FLAG_WEAPON)
		{
			if (m_properties[Data::Item::PROPERTY_TYPE_WEAPON_DAMAGE_MIN] == 0 || m_properties[Data::Item::PROPERTY_TYPE_WEAPON_DAMAGE_MAX] == 0)
			{
				// This weapon doesn't have explicit damage range, use defaults based on item level
				bool isTwoHanded = itemTypeInfo->m_flags & ItemType::FLAG_TWO_HANDED;
				uint32_t dps = isTwoHanded ? aManifest->m_itemMetrics.GetLevelBase2HWeaponDPS(itemLevel) : aManifest->m_itemMetrics.GetLevelBase1HWeaponDPS(itemLevel);
				uint32_t cooldown = GetWeaponCooldown();
				uint32_t avgDamage = (dps * cooldown) / 10;
				uint32_t minDamage = avgDamage / 2;
				uint32_t maxDamage = (avgDamage * 3) / 2;
				
				if(minDamage == 0)
					minDamage = 1;

				if(maxDamage <= minDamage)
					maxDamage = minDamage + 1;

				m_properties[Data::Item::PROPERTY_TYPE_WEAPON_DAMAGE_MIN] = minDamage;
				m_properties[Data::Item::PROPERTY_TYPE_WEAPON_DAMAGE_MAX] = maxDamage;
			}
		}

		bool isArmor = itemTypeInfo->m_flags & ItemType::FLAG_ARMOR;

		if(isArmor || m_properties[Data::Item::PROPERTY_TYPE_COST] == 0)
		{
			ItemMetrics::Multipliers itemTypeMultipliers = aManifest->m_itemMetrics.GetItemTypeMultipliers(m_itemData->m_itemType);
			ItemMetrics::Multipliers equipmentSlotMultipliers = aManifest->m_itemMetrics.GetEquipmentSlotMultipliers(m_itemData->m_equipmentSlots);

			ItemMetrics::Multipliers multipliers;
			multipliers.m_armor = itemTypeMultipliers.m_armor * equipmentSlotMultipliers.m_armor;
			multipliers.m_cost = itemTypeMultipliers.m_cost * equipmentSlotMultipliers.m_cost;

			if(isArmor)
			{
				uint32_t baseArmor = aManifest->m_itemMetrics.GetLevelBaseArmor(itemLevel);
				baseArmor = 1 + (uint32_t)((float)baseArmor * multipliers.m_armor);
				m_stats.m_stats[Stat::ID_ARMOR] += baseArmor; // Add it
			}

			if(m_properties[Data::Item::PROPERTY_TYPE_COST] == 0)
			{
				uint32_t baseCost = aManifest->m_itemMetrics.GetLevelBaseCost(itemLevel);
				baseCost = 1 + (uint32_t)((float)baseCost * multipliers.m_cost);
				m_properties[Data::Item::PROPERTY_TYPE_COST] = baseCost;
			}
		}

		m_vendorValue = (uint32_t)((float)m_properties[Data::Item::PROPERTY_TYPE_COST] * aManifest->m_itemMetrics.m_vendorCostMultiplier);
	}
	
	ItemInstanceData::~ItemInstanceData()
	{

	}

	uint32_t	
	ItemInstanceData::GetItemLevel() const
	{
		if(m_itemData->m_itemType != 0)
			return m_itemData->m_itemType;
		return m_itemData->m_requiredLevel;
	}

	uint32_t	
	ItemInstanceData::GetWeaponCooldown() const
	{
		uint32_t weaponCooldown = m_properties[Data::Item::PROPERTY_TYPE_WEAPON_COOLDOWN];
		if(weaponCooldown == 0)
			weaponCooldown = 20;
		return weaponCooldown;
	}

	//---------------------------------------------------------

	void		
	ItemInstanceData::_Generate(
		std::mt19937&			aRandom,
		const Manifest*			aManifest,
		const Data::Item::Node*	aNode)
	{
		if(!aNode->m_name.empty())
			m_name = aNode->m_name;

		if (!aNode->m_suffix.empty())
			m_suffix = aNode->m_suffix;

		if(aNode->m_iconSpriteId != 0)
			m_iconSpriteId = aNode->m_iconSpriteId;

		for(const Data::Item::Property& p : aNode->m_properties)
			m_properties[p.m_type] = p.m_value;

		for(const Data::Item::AddedStat& addedStat : aNode->m_addedStats)
			m_stats.m_stats[addedStat.m_id] += Helpers::RandomInRange<uint32_t>(aRandom, addedStat.m_min, addedStat.m_max);

		for(const std::unique_ptr<Data::Item::Node>& child : aNode->m_randomChildren)
		{
			if(aRandom() <= child->m_chance)
				_Generate(aRandom, aManifest, child.get());
		}

		if(aNode->m_weightedChildren.size() > 0)
		{
			uint32_t sum = 0;
			uint32_t roll = Helpers::RandomInRange<uint32_t>(aRandom, 0, aNode->m_totalChildWeight);
			const Data::Item::Node* chosen = NULL;

			for(size_t i = 0; i < aNode->m_weightedChildren.size() && chosen == NULL; i++)
			{
				const std::unique_ptr<Data::Item::Node>& child = aNode->m_weightedChildren[i];
				sum += child->m_weight;
				if(roll <= sum)
					chosen = child.get();
			}

			assert(chosen != NULL);

			_Generate(aRandom, aManifest, chosen);
		}

		m_isWeapon = m_properties[Data::Item::PROPERTY_TYPE_WEAPON_COOLDOWN] != 0;

		uint32_t& rarity = m_properties[Data::Item::PROPERTY_TYPE_RARITY];
		if(rarity == 0)
			rarity = 1; // Must always have a rarity
	}

}