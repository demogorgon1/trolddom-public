#pragma once

#include "EquipmentSlot.h"
#include "IReader.h"
#include "ItemType.h"
#include "IWriter.h"
#include "Parser.h"
#include "Rarity.h"

namespace tpublic
{

	class ItemMetrics
	{
	public:
		struct InvalidStatsItemType
		{
			InvalidStatsItemType()
			{

			}

			InvalidStatsItemType(
				const SourceNode*		aSource)
			{
				m_itemTypeId = ItemType::StringToId(aSource->m_name.c_str());
				TP_VERIFY(m_itemTypeId != ItemType::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid item type.", aSource->m_name.c_str());

				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "ignore_equipment_slot")
					{
						m_ignoreEquipmentSlotId = EquipmentSlot::StringToId(aChild->GetIdentifier());
						TP_VERIFY(m_ignoreEquipmentSlotId != EquipmentSlot::INVALID_ID, aChild->m_debugInfo, "'%s' is not a equipment slot.", aChild->GetIdentifier());
					}
					else
					{
						Stat::Id replacementStatId = Stat::StringToId(aChild->m_name.c_str());
						TP_VERIFY(replacementStatId != Stat::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid stat.", aChild->m_name.c_str());

						aChild->GetArray()->ForEachChild([&](
							const SourceNode* aItem)
						{
							Stat::Id illegalStatId = Stat::StringToId(aItem->GetIdentifier());
							TP_VERIFY(illegalStatId != Stat::INVALID_ID, aItem->m_debugInfo, "'%s' is not a valid stat.", aItem->GetIdentifier());
							m_table[(uint32_t)illegalStatId] = (uint32_t)replacementStatId;
						});
					}
				});
			}

			void
			ToStream(
				IWriter*				aWriter) const
			{
				aWriter->WritePOD(m_itemTypeId);
				aWriter->WritePOD(m_ignoreEquipmentSlotId);
				aWriter->WriteUIntToUIntTable<uint32_t, uint32_t>(m_table);
			}

			bool
			FromStream(
				IReader*				aReader)
			{
				if (!aReader->ReadPOD(m_itemTypeId))
					return false;
				if (!aReader->ReadPOD(m_ignoreEquipmentSlotId))
					return false;
				if(!aReader->ReadUIntToUIntTable(m_table))
					return false;
				return true;
			}

			// Public data
			ItemType::Id						m_itemTypeId = ItemType::INVALID_ID;
			EquipmentSlot::Id					m_ignoreEquipmentSlotId = EquipmentSlot::INVALID_ID;

			typedef std::unordered_map<uint32_t, uint32_t> Table;
			Table								m_table;
		};

		struct InvalidStats
		{
			void
			FromSource(
				const SourceNode*		aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if (aChild->m_tag == "item_type")
						m_itemTypes.push_back(std::make_unique<InvalidStatsItemType>(aChild));
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*				aWriter) const
			{
				aWriter->WriteObjectPointers(m_itemTypes);
			}

			bool
			FromStream(
				IReader*				aReader)
			{
				if(!aReader->ReadObjectPointers(m_itemTypes))
					return false;
				return true;
			}

			const InvalidStatsItemType*
			GetItemType(
				ItemType::Id			aItemTypeId) const
			{
				for(const std::unique_ptr<InvalidStatsItemType>& t : m_itemTypes)
				{
					if(t->m_itemTypeId == aItemTypeId)
						return t.get();
				}
				return NULL;
			}

			bool
			ShouldReplaceStat(
				ItemType::Id					aItemTypeId,
				const std::vector<uint32_t>&	aEquipmentSlots,
				Stat::Id						aStatId,
				uint32_t						aValue,
				Stat::Id&						aOutStatId,
				uint32_t&						aOutValue) const
			{
				const InvalidStatsItemType* itemType = GetItemType(aItemTypeId);
				if(itemType == NULL)
					return false;

				InvalidStatsItemType::Table::const_iterator i = itemType->m_table.find((uint32_t)aStatId);
				if(i == itemType->m_table.cend())
					return false;

				if(itemType->m_ignoreEquipmentSlotId != EquipmentSlot::INVALID_ID)
				{
					if(Helpers::FindItem(aEquipmentSlots, (uint32_t)itemType->m_ignoreEquipmentSlotId) != SIZE_MAX)
						return false;
				}

				aOutStatId = (Stat::Id)i->second;
				const Stat::Info* replacementStatInfo = Stat::GetInfo(aOutStatId);
				const Stat::Info* illegalStatInfo = Stat::GetInfo(aStatId);
				aOutValue = (uint32_t)(((float)aValue * illegalStatInfo->m_budgetCost) / replacementStatInfo->m_budgetCost);

				if(aOutValue == 0)
					aOutValue = 1;
				return true;
			}
			
			// Public data
			std::vector<std::unique_ptr<InvalidStatsItemType>>	m_itemTypes;
		};

		struct Multipliers
		{
			void
			FromSource(
				const SourceNode*		aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if (aChild->m_name == "armor")
						m_armor = aChild->GetFloat();
					else if (aChild->m_name == "cost")
						m_cost = aChild->GetFloat();
					else if (aChild->m_name == "stat_budget")
						m_statBudget = aChild->GetFloat();
					else if (aChild->m_name == "weapon_damage")
						m_weaponDamage = aChild->GetFloat();
					else if (aChild->m_name == "token_cost")
						m_tokenCost = aChild->GetFloat();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*				aWriter) const
			{
				aWriter->WriteFloat(m_armor);
				aWriter->WriteFloat(m_cost);
				aWriter->WriteFloat(m_statBudget);
				aWriter->WriteFloat(m_weaponDamage);
				aWriter->WriteFloat(m_tokenCost);
			}

			bool
			FromStream(
				IReader*				aReader) 
			{
				if (!aReader->ReadFloat(m_armor))
					return false;
				if (!aReader->ReadFloat(m_cost))
					return false;
				if (!aReader->ReadFloat(m_statBudget))
					return false;
				if (!aReader->ReadFloat(m_weaponDamage))
					return false;
				if (!aReader->ReadFloat(m_tokenCost))
					return false;
				return true;
			}

			// Public data
			float	m_armor = 1.0f;
			float	m_cost = 1.0f;
			float	m_statBudget = 1.0f;
			float	m_weaponDamage = 1.0f;
			float	m_tokenCost = 1.0f;
		};

		ItemMetrics()
		{

		}

		void
		FromSource(
			const SourceNode*			aSource)
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "base_armor")
				{
					aChild->GetArray()->ForEachChild([&](
						const SourceNode* aEntry)
					{
						TP_VERIFY(aEntry->m_type == SourceNode::TYPE_ARRAY && aEntry->m_children.size() == 2, aEntry->m_debugInfo, "Not a level-armor pair.");
						uint32_t level = aEntry->m_children[0]->GetUInt32();
						uint32_t armor = aEntry->m_children[1]->GetUInt32();

						if(level >= m_levelBaseArmor.size())
							m_levelBaseArmor.resize(level + 1);

						m_levelBaseArmor[level] = armor;
					});
				}
				else if(aChild->m_name == "base_cost")
				{
					aChild->GetArray()->ForEachChild([&](
						const SourceNode* aEntry)
					{
						TP_VERIFY(aEntry->m_type == SourceNode::TYPE_ARRAY && aEntry->m_children.size() == 2, aEntry->m_debugInfo, "Not a level-cost pair.");
						uint32_t level = aEntry->m_children[0]->GetUInt32();
						uint32_t cost = aEntry->m_children[1]->GetUInt32();

						if(level >= m_levelBaseCost.size())
							m_levelBaseCost.resize(level + 1);

						m_levelBaseCost[level] = cost;
					});
				}
				else if(aChild->m_name == "base_1h_weapon_dps")
				{
					aChild->GetArray()->ForEachChild([&](
						const SourceNode* aEntry)
					{
						TP_VERIFY(aEntry->m_type == SourceNode::TYPE_ARRAY && aEntry->m_children.size() == 2, aEntry->m_debugInfo, "Not a level-dps pair.");
						uint32_t level = aEntry->m_children[0]->GetUInt32();
						uint32_t dps = aEntry->m_children[1]->GetUInt32();

						if(level >= m_levelBase1HWeaponDPS.size())
							m_levelBase1HWeaponDPS.resize(level + 1);

						m_levelBase1HWeaponDPS[level] = dps;
					});
				}
				else if(aChild->m_name == "base_2h_weapon_dps")
				{
					aChild->GetArray()->ForEachChild([&](
						const SourceNode* aEntry)
					{
						TP_VERIFY(aEntry->m_type == SourceNode::TYPE_ARRAY && aEntry->m_children.size() == 2, aEntry->m_debugInfo, "Not a level-dps pair.");
						uint32_t level = aEntry->m_children[0]->GetUInt32();
						uint32_t dps = aEntry->m_children[1]->GetUInt32();

						if(level >= m_levelBase2HWeaponDPS.size())
							m_levelBase2HWeaponDPS.resize(level + 1);

						m_levelBase2HWeaponDPS[level] = dps;
					});
				}
				else if(aChild->m_name == "base_ranged_dps")
				{
					aChild->GetArray()->ForEachChild([&](
						const SourceNode* aEntry)
					{
						TP_VERIFY(aEntry->m_type == SourceNode::TYPE_ARRAY && aEntry->m_children.size() == 2, aEntry->m_debugInfo, "Not a level-dps pair.");
						uint32_t level = aEntry->m_children[0]->GetUInt32();
						uint32_t dps = aEntry->m_children[1]->GetUInt32();

						if(level >= m_levelBaseRangedDPS.size())
							m_levelBaseRangedDPS.resize(level + 1);

						m_levelBaseRangedDPS[level] = dps;
					});
				}
				else if(aChild->m_tag == "item_type_multipliers")
				{
					ItemType::Id itemType = ItemType::StringToId(aChild->m_name.c_str());
					TP_VERIFY(itemType != ItemType::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid item type.", aChild->m_name.c_str());
					m_itemTypeMultipliers[itemType].FromSource(aChild);
				}
				else if (aChild->m_tag == "equipment_slot_multipliers")
				{
					EquipmentSlot::Id equipmentSlot = EquipmentSlot::StringToId(aChild->m_name.c_str());
					TP_VERIFY(equipmentSlot != EquipmentSlot::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid equipment slot.", aChild->m_name.c_str());
					m_equipmentSlotMultipliers[equipmentSlot].FromSource(aChild);
				}
				else if (aChild->m_tag == "rarity_multipliers")
				{
					Rarity::Id rarity = Rarity::StringToId(aChild->m_name.c_str());
					TP_VERIFY(rarity != Rarity::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid rarity.", aChild->m_name.c_str());
					m_rarityMultipliers[rarity].FromSource(aChild);
				}
				else if(aChild->m_name == "vendor_cost_multiplier")
				{
					m_vendorCostMultiplier = aChild->GetFloat();
				}
				else if (aChild->m_name == "token_cost_base_multiplier")
				{
					m_tokenCostBaseMultiplier = aChild->GetFloat();
				}
				else if (aChild->m_name == "shield_armor_to_base_block_value")
				{
					m_shieldArmorToBaseBlockValue = aChild->GetFloat();
				}
				else if (aChild->m_name == "offhand_dps_multiplier")
				{
					m_offHandDPSMultiplier = aChild->GetFloat();
				}
				else if(aChild->m_name == "invalid_stats")
				{
					m_invalidStats.FromSource(aChild);
				}
				else
				{
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				}
			});
		}

		void
		ToStream(
			IWriter*						aStream) const 
		{
			aStream->WriteUInts(m_levelBaseArmor);
			aStream->WriteUInts(m_levelBaseCost);
			aStream->WriteUInts(m_levelBase1HWeaponDPS);
			aStream->WriteUInts(m_levelBase2HWeaponDPS);
			aStream->WriteUInts(m_levelBaseRangedDPS);
			aStream->WriteFloat(m_vendorCostMultiplier);
			aStream->WriteFloat(m_offHandDPSMultiplier);
			aStream->WriteFloat(m_shieldArmorToBaseBlockValue);
			aStream->WriteFloat(m_tokenCostBaseMultiplier);
			m_invalidStats.ToStream(aStream);

			for(uint32_t i = 1; i < (uint32_t)ItemType::NUM_IDS; i++)
				m_itemTypeMultipliers[i].ToStream(aStream);

			for (uint32_t i = 1; i < (uint32_t)EquipmentSlot::NUM_IDS; i++)
				m_equipmentSlotMultipliers[i].ToStream(aStream);

			for (uint32_t i = 1; i < (uint32_t)Rarity::NUM_IDS; i++)
				m_rarityMultipliers[i].ToStream(aStream);
		}

		bool
		FromStream(
			IReader*						aStream) 
		{
			if(!aStream->ReadUInts(m_levelBaseArmor))
				return false;
			if (!aStream->ReadUInts(m_levelBaseCost))
				return false;
			if (!aStream->ReadUInts(m_levelBase1HWeaponDPS))
				return false;
			if (!aStream->ReadUInts(m_levelBase2HWeaponDPS))
				return false;
			if (!aStream->ReadUInts(m_levelBaseRangedDPS))
				return false;
			if (!aStream->ReadFloat(m_vendorCostMultiplier))
				return false;
			if (!aStream->ReadFloat(m_offHandDPSMultiplier))
				return false;
			if (!aStream->ReadFloat(m_shieldArmorToBaseBlockValue))
				return false;
			if (!aStream->ReadFloat(m_tokenCostBaseMultiplier))
				return false;
			if(!m_invalidStats.FromStream(aStream))
				return false;

			for (uint32_t i = 1; i < (uint32_t)ItemType::NUM_IDS; i++)
			{
				if (!m_itemTypeMultipliers[i].FromStream(aStream))
					return false;
			}

			for (uint32_t i = 1; i < (uint32_t)EquipmentSlot::NUM_IDS; i++)
			{
				if (!m_equipmentSlotMultipliers[i].FromStream(aStream))
					return false;
			}

			for (uint32_t i = 1; i < (uint32_t)Rarity::NUM_IDS; i++)
			{
				if (!m_rarityMultipliers[i].FromStream(aStream))
					return false;
			}

			return true;
		}
		
		uint32_t
		GetLevelBaseArmor(
			uint32_t						aLevel) const
		{
			if(aLevel >= m_levelBaseArmor.size())
				return 1;
			return m_levelBaseArmor[aLevel];
		}

		uint32_t
		GetLevelBaseCost(
			uint32_t						aLevel) const
		{
			if (aLevel >= m_levelBaseCost.size())
				return 1;
			return m_levelBaseCost[aLevel];
		}

		uint32_t
		GetLevelBase1HWeaponDPS(
			uint32_t						aLevel) const
		{
			if (aLevel >= m_levelBase1HWeaponDPS.size())
				return 1;
			return m_levelBase1HWeaponDPS[aLevel];
		}

		uint32_t
		GetLevelBase2HWeaponDPS(
			uint32_t						aLevel) const
		{
			if (aLevel >= m_levelBase2HWeaponDPS.size())
				return 1;
			return m_levelBase2HWeaponDPS[aLevel];
		}

		uint32_t
		GetLevelBaseRangedDPS(
			uint32_t						aLevel) const
		{
			if (aLevel >= m_levelBaseRangedDPS.size())
				return 1;
			return m_levelBaseRangedDPS[aLevel];
		}

		const Multipliers&
		GetItemTypeMultipliers(
			ItemType::Id					aItemType) const
		{
			return m_itemTypeMultipliers[aItemType];
		}

		Multipliers
		GetEquipmentSlotMultipliers(
			const std::vector<uint32_t>&	aEquipmentSlots) const
		{
			if(aEquipmentSlots.size() == 0)
				return Multipliers();

			Multipliers t;

			for(uint32_t equipmentSlot : aEquipmentSlots)
			{
				const Multipliers& multipliers = m_equipmentSlotMultipliers[equipmentSlot];
				if (multipliers.m_armor > t.m_armor)
					t.m_armor = multipliers.m_armor;
				if (multipliers.m_cost > t.m_cost)
					t.m_cost = multipliers.m_cost;
			}
			
			return t;
		}

		const Multipliers&
		GetRarityMultipliers(
			Rarity::Id						aRarity) const
		{
			return m_rarityMultipliers[aRarity];
		}
		
		// Public data
		std::vector<uint32_t>					m_levelBaseArmor;
		std::vector<uint32_t>					m_levelBaseCost;
		std::vector<uint32_t>					m_levelBase1HWeaponDPS;
		std::vector<uint32_t>					m_levelBase2HWeaponDPS;
		std::vector<uint32_t>					m_levelBaseRangedDPS;
		Multipliers								m_itemTypeMultipliers[ItemType::NUM_IDS];
		Multipliers								m_equipmentSlotMultipliers[EquipmentSlot::NUM_IDS];
		Multipliers								m_rarityMultipliers[Rarity::NUM_IDS];
		float									m_vendorCostMultiplier = 0.5f;
		float									m_shieldArmorToBaseBlockValue = 0.0f;
		float									m_offHandDPSMultiplier = 0.5f;
		float									m_tokenCostBaseMultiplier = 0.1f;
		InvalidStats							m_invalidStats;
	};

}
