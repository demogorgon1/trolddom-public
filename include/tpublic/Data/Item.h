#pragma once

#include "../DataBase.h"
#include "../EquipmentSlot.h"
#include "../ItemBinding.h"
#include "../ItemType.h"
#include "../Rarity.h"
#include "../Stat.h"
#include "../UIntRange.h"

namespace tpublic
{

	namespace Data
	{

		struct Item
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ITEM;
			static const bool TAGGED = true;

			struct AddedStat
			{
				AddedStat()
				{

				}

				AddedStat(					
					const SourceNode*	aSource,
					bool				aIsBudgetWeight)
					: m_isBudgetWeight(aIsBudgetWeight)
				{
					m_id = Stat::StringToId(aSource->m_name.c_str());
					TP_VERIFY(m_id != Stat::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid stat.", aSource->m_name.c_str());
					m_value = aSource->GetUInt32();
				}

				void 
				ToStream(
					IWriter*			aStream) const
				{
					aStream->WritePOD(m_id);
					aStream->WriteBool(m_isBudgetWeight);
					aStream->WriteUInt(m_value);
				}

				bool
				FromStream(
					IReader*			aStream)
				{
					if (!aStream->ReadPOD(m_id))
						return false;
					if (!aStream->ReadBool(m_isBudgetWeight))
						return false;
					if(!aStream->ReadUInt(m_value))
						return false;
					return true;
				}

				// Public data
				Stat::Id			m_id = Stat::Id(0);
				bool				m_isBudgetWeight = false;
				uint32_t			m_value = 0;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			bool
			IsEquippableInSlot(
				uint32_t				aSlot) const
			{
				for(uint32_t slot : m_equipmentSlots)
				{
					if(slot == aSlot)
						return true;
				}
				return false;
			}				

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "equipment_slots")
						{
							aChild->ForEachChild([&](
								const SourceNode* aEquipmentSlot)
							{
								EquipmentSlot::Id id = EquipmentSlot::StringToId(aEquipmentSlot->GetIdentifier());
								TP_VERIFY(id != EquipmentSlot::INVALID_ID, aEquipmentSlot->m_debugInfo, "'%s' is not a valid equipment slot.", aEquipmentSlot->GetIdentifier());
								m_equipmentSlots.push_back(id);
							});
						}
						else if (aChild->m_name == "loot_groups")
						{
							aChild->GetIdArray(DataType::ID_LOOT_GROUP, m_lootGroups);
						}
						else if (aChild->m_name == "stack")
						{
							m_stackSize = aChild->GetUInt32();
						}
						else if (aChild->m_name == "item_level")
						{
							m_itemLevel = aChild->GetUInt32();
						}
						else if (aChild->m_name == "level_range")
						{
							m_levelRange = UIntRange(aChild);
						}
						else if (aChild->m_name == "required_level")
						{
							m_requiredLevel = aChild->GetUInt32();
						}
						else if (aChild->m_name == "cost")
						{
							m_cost = aChild->GetUInt32();
						}
						else if (aChild->m_name == "unique")
						{
							m_unique = aChild->GetBool();
						}
						else if (aChild->m_name == "use_ability")
						{
							m_useAbilityId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY, aChild->GetIdentifier());
						}
						else if (aChild->m_name == "icon")
						{
							m_iconSpriteId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->GetIdentifier());
						}
						else if (aChild->m_name == "rarity")
						{
							m_rarity = Rarity::StringToId(aChild->GetIdentifier());
							TP_VERIFY(m_rarity != Rarity::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid rarity.", aChild->GetIdentifier());
						}
						else if (aChild->m_name == "type")
						{
							m_itemType = ItemType::StringToId(aChild->GetIdentifier());
							TP_VERIFY(m_itemType != ItemType::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid item type.", aChild->GetIdentifier());
						}
						else if (aChild->m_name == "binds")
						{
							m_itemBinding = ItemBinding::StringToId(aChild->GetIdentifier());
							TP_VERIFY(m_itemBinding != ItemBinding::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid item binding.", aChild->GetIdentifier());
						}
						else if (aChild->m_name == "weapon_cooldown")
						{
							m_weaponCooldown = aChild->GetUInt32();
						}
						else if (aChild->m_name == "weapon_damage")
						{
							m_weaponDamage = UIntRange(aChild);
						}
						else if (aChild->m_name == "string")
						{
							m_string = aChild->GetString();
						}
						else if (aChild->m_name == "flavor")
						{
							m_flavor = aChild->GetString();
						}
						else if (aChild->m_name == "budget_bias")
						{
							m_budgetBias = aChild->GetInt32();
						}
						else if (aChild->m_tag == "stat")
						{
							m_addedStats.push_back(AddedStat(aChild, false));
						}
						else if (aChild->m_tag == "stat_weight")
						{
							m_addedStats.push_back(AddedStat(aChild, true));
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					}
				});

				if(m_itemLevel == 0)
					m_itemLevel = m_requiredLevel;
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInts(m_equipmentSlots);
				aStream->WriteUInts(m_lootGroups);
				aStream->WriteUInt(m_stackSize);
				aStream->WriteUInt(m_useAbilityId);
				aStream->WritePOD(m_itemType);
				m_levelRange.ToStream(aStream);
				aStream->WriteUInt(m_requiredLevel);
				aStream->WriteUInt(m_itemLevel);
				aStream->WriteUInt(m_iconSpriteId);
				aStream->WriteUInt(m_cost);
				aStream->WritePOD(m_rarity);
				aStream->WriteBool(m_unique);
				aStream->WriteObjects(m_addedStats);
				aStream->WriteOptionalObject(m_weaponDamage);
				aStream->WriteUInt(m_weaponCooldown);
				aStream->WriteString(m_string);
				aStream->WriteString(m_flavor);
				aStream->WriteInt(m_budgetBias);
				aStream->WritePOD(m_itemBinding);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInts(m_equipmentSlots))
					return false;
				if (!aStream->ReadUInts(m_lootGroups))
					return false;
				if (!aStream->ReadUInt(m_stackSize))
					return false;
				if (!aStream->ReadUInt(m_useAbilityId))
					return false;
				if(!aStream->ReadPOD(m_itemType))
					return false;
				if(!m_levelRange.FromStream(aStream))
					return false;
				if (!aStream->ReadUInt(m_requiredLevel))
					return false;
				if (!aStream->ReadUInt(m_itemLevel))
					return false;
				if (!aStream->ReadUInt(m_iconSpriteId))
					return false;
				if (!aStream->ReadUInt(m_cost))
					return false;
				if (!aStream->ReadPOD(m_rarity))
					return false;
				if(!aStream->ReadBool(m_unique))
					return false;
				if (!aStream->ReadObjects(m_addedStats))
					return false;
				if (!aStream->ReadOptionalObject(m_weaponDamage))
					return false;
				if (!aStream->ReadUInt(m_weaponCooldown))
					return false;
				if (!aStream->ReadString(m_string))
					return false;
				if (!aStream->ReadString(m_flavor))
					return false;
				if (!aStream->ReadInt(m_budgetBias))
					return false;
				if (!aStream->ReadPOD(m_itemBinding))
					return false;
				return true;
			}

			// Public data
			std::vector<uint32_t>		m_equipmentSlots;
			std::vector<uint32_t>		m_lootGroups;
			uint32_t					m_stackSize = 1;
			uint32_t					m_useAbilityId = 0;
			ItemType::Id				m_itemType = ItemType::ID_NONE;
			UIntRange					m_levelRange;
			uint32_t					m_requiredLevel = 0;
			uint32_t					m_itemLevel = 0;
			uint32_t					m_iconSpriteId = 0;
			uint32_t					m_cost = 0;
			Rarity::Id					m_rarity = Rarity::ID_COMMON;
			bool						m_unique = false;
			std::vector<AddedStat>		m_addedStats;
			std::optional<UIntRange>	m_weaponDamage;
			uint32_t					m_weaponCooldown = 0;
			std::string					m_string;
			std::string					m_flavor;
			int32_t						m_budgetBias = 0;
			ItemBinding::Id				m_itemBinding = ItemBinding::ID_NEVER;
		};

	}

}