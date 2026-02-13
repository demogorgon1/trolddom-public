#include "../Pcheader.h"

#include <tpublic/Data/Item.h>

#include <tpublic/ItemMetrics.h>
#include <tpublic/Manifest.h>

namespace tpublic::Data
{

	void
	Item::ReadSource(
		const SourceNode*		aSource)
	{
		if (aSource->m_name == "equipment_slots")
		{
			aSource->ForEachChild([&](
				const SourceNode* aEquipmentSlot)
			{
				EquipmentSlot::Id id = EquipmentSlot::StringToId(aEquipmentSlot->GetIdentifier());
				TP_VERIFY(id != EquipmentSlot::INVALID_ID, aEquipmentSlot->m_debugInfo, "'%s' is not a valid equipment slot.", aEquipmentSlot->GetIdentifier());
				m_equipmentSlots.push_back(id);
			});
		}
		else if (aSource->m_name == "loot_groups")
		{
			aSource->GetIdArray(DataType::ID_LOOT_GROUP, m_lootGroups);
		}
		else if (aSource->m_name == "stack")
		{
			m_stackSize = aSource->GetUInt32();
		}
		else if (aSource->m_name == "item_level")
		{
			m_itemLevel = aSource->GetUInt32();
		}
		else if (aSource->m_name == "level_range")
		{
			m_levelRange = UIntRange(aSource);
		}
		else if (aSource->m_name == "required_level")
		{
			m_requiredLevel = aSource->GetUInt32();
		}
		else if (aSource->m_name == "cost")
		{
			m_cost = aSource->GetUInt32();
		}
		else if (aSource->m_name == "use_ability")
		{
			m_useAbilityId = aSource->GetId(DataType::ID_ABILITY);
		}
		else if (aSource->m_name == "icon")
		{
			m_iconSpriteId = aSource->GetId(DataType::ID_SPRITE);
		}
		else if (aSource->m_name == "quest")
		{
			m_questId = aSource->GetId(DataType::ID_QUEST);
		}
		else if (aSource->m_name == "aura")
		{
			m_auraId = aSource->GetId(DataType::ID_AURA);
		}
		else if (aSource->m_name == "rarity")
		{
			m_rarity = Rarity::StringToId(aSource->GetIdentifier());
			TP_VERIFY(m_rarity != Rarity::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid rarity.", aSource->GetIdentifier());
		}
		else if (aSource->m_name == "type")
		{
			m_itemType = ItemType::StringToId(aSource->GetIdentifier());
			TP_VERIFY(m_itemType != ItemType::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid item type.", aSource->GetIdentifier());
		}
		else if (aSource->m_name == "binds")
		{
			m_itemBinding = ItemBinding::StringToId(aSource->GetIdentifier());
			TP_VERIFY(m_itemBinding != ItemBinding::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid item binding.", aSource->GetIdentifier());
		}
		else if (aSource->m_name == "weapon_cooldown")
		{
			m_weaponCooldown = aSource->GetUInt32();
		}
		else if (aSource->m_name == "bag_slots")
		{
			m_bagSlots = aSource->GetUInt32();
		}
		else if (aSource->m_name == "token_cost")
		{
			m_tokenCost = aSource->GetUInt32();
		}
		else if (aSource->m_name == "empower")
		{
			m_empower = aSource->GetUInt32();
		}
		else if (aSource->m_name == "weapon_damage")
		{
			m_weaponDamage = UIntRange(aSource);
		}
		else if (aSource->m_name == "string")
		{
			m_string = aSource->GetString();
		}
		else if (aSource->m_name == "flags")
		{
			m_flags |= SourceToFlags(aSource);
		}
		else if (aSource->m_name == "flavor")
		{
			m_flavor = aSource->GetString();
		}
		else if (aSource->m_name == "budget_bias")
		{
			m_budgetBias = aSource->GetInt32();
		}
		else if (aSource->m_tag == "stat")
		{
			m_addedStats.push_back(AddedStat(aSource, false));
		}
		else if (aSource->m_tag == "stat_weight")
		{
			m_addedStats.push_back(AddedStat(aSource, true));
		}
		else if (aSource->m_name == "sound_effects")
		{
			m_soundEffects.FromSource(aSource);
		}
		else if (aSource->m_name == "value_multiplier")
		{
			m_valueMultiplier = aSource->GetFloat();
		}
		else if (aSource->m_name == "armor_style_visual")
		{
			m_armorStyleVisual = ArmorStyle::Visual(aSource);
		}
		else if (aSource->m_name == "oracle")
		{
			m_oracle = std::make_unique<Oracle>(aSource);
		}
		else if (aSource->m_tag == "variant")
		{
			m_variants.push_back(std::make_unique<Variant>(aSource));
		}
		else if(aSource->m_name == "variants")
		{
			aSource->GetObject()->ForEachChild([&](
				const SourceNode* aChild)
			{
				TP_VERIFY(aChild->m_tag == "variant", aChild->m_debugInfo, "Variant expected.");
				m_variants.push_back(std::make_unique<Variant>(aChild));
			});
		}
		else
		{
			TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid item.", aSource->m_name.c_str());
		}
	}

	void	
	Item::InitVariant(
		const Item*				aVariant)
	{
		// FIXME: warn if variant contains stuff that won't be carried over

		if(!aVariant->m_lootGroups.empty())
			m_lootGroups = aVariant->m_lootGroups;

		if(aVariant->m_useAbilityId != 0)
			m_useAbilityId = aVariant->m_useAbilityId;

		if (aVariant->m_levelRange.m_min != 0 || aVariant->m_levelRange.m_max != 0)
			m_levelRange = aVariant->m_levelRange;

		if (aVariant->m_requiredLevel != 0)
			m_requiredLevel = aVariant->m_requiredLevel;

		if (aVariant->m_itemLevel != 0)
			m_itemLevel = aVariant->m_itemLevel;

		if (aVariant->m_iconSpriteId != 0)
			m_iconSpriteId = aVariant->m_iconSpriteId;

		if (aVariant->m_cost != 0)
			m_cost = aVariant->m_cost;

		if (aVariant->m_weaponDamage)
			m_weaponDamage = aVariant->m_weaponDamage;

		if (aVariant->m_weaponCooldown != 0)
			m_weaponCooldown = aVariant->m_weaponCooldown;

		if (!aVariant->m_string.empty())
			m_string = aVariant->m_string;

		if (!aVariant->m_flavor.empty())
			m_flavor = aVariant->m_flavor;

		if (aVariant->m_budgetBias != 0)
			m_budgetBias = aVariant->m_budgetBias;

		if (aVariant->m_flags != 0)
			m_flags |= aVariant->m_flags; // FIXME: in some situations we might want to override instead?

		if (aVariant->m_auraId != 0)
			m_auraId = aVariant->m_auraId;

		if (aVariant->m_empower != 0)
			m_empower = aVariant->m_empower;

		for(const AddedStat& addedStat : aVariant->m_addedStats)
		{
			AddedStat* existing = NULL;
			for(AddedStat& t : m_addedStats)
			{
				if(t.m_id == addedStat.m_id && t.m_isBudgetWeight == addedStat.m_isBudgetWeight)
				{
					existing = &t;
					break;
				}
			}

			if(existing != NULL)
				existing->m_value += addedStat.m_value;
			else 
				m_addedStats.push_back(addedStat);
		}
	}

	//-----------------------------------------------------------------------------------------------

	void
	Item::PrepareRuntime(
		uint8_t					/*aRuntime*/,
		const Manifest*			aManifest)
	{
		std::vector<AddedStat> replacementStats;

		for (size_t i = 0; i < m_addedStats.size(); i++)
		{
			const AddedStat& addedStat = m_addedStats[i];

			AddedStat replacementStat;
			if (aManifest->m_itemMetrics->m_invalidStats.ShouldReplaceStat(m_itemType, m_equipmentSlots, addedStat.m_id, addedStat.m_value, replacementStat.m_id, replacementStat.m_value))
			{
				// Budget weights are just transfered directly while raw stats need to be converted based on cost
				if (addedStat.m_isBudgetWeight)
				{
					replacementStat.m_value = addedStat.m_value;
					replacementStat.m_isBudgetWeight = true;
				}

				replacementStats.push_back(replacementStat);

				m_addedStats.erase(m_addedStats.begin() + i);
				i--;
			}
		}

		for (const AddedStat& replacementStat : replacementStats)
		{
			// Already has this one? 
			bool alreadyThere = false;
			for (AddedStat& addedStat : m_addedStats)
			{
				if (addedStat.m_id == replacementStat.m_id && addedStat.m_isBudgetWeight == replacementStat.m_isBudgetWeight)
				{
					addedStat.m_value += replacementStat.m_value;
					alreadyThere = true;
					break;
				}
			}

			if (!alreadyThere)
				m_addedStats.push_back(replacementStat);
		}
	}
}