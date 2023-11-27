#pragma once

#include "../DataBase.h"
#include "../EquipmentSlot.h"
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
					m_range = UIntRange(aSource);
				}

				void 
				ToStream(
					IWriter*			aStream) const
				{
					aStream->WritePOD(m_id);
					aStream->WriteBool(m_isBudgetWeight);
					m_range.ToStream(aStream);
				}

				bool
				FromStream(
					IReader*			aStream)
				{
					if (!aStream->ReadPOD(m_id))
						return false;
					if (!aStream->ReadBool(m_isBudgetWeight))
						return false;
					if(!m_range.FromStream(aStream))
						return false;
					return true;
				}

				// Public data
				Stat::Id			m_id = Stat::Id(0);
				bool				m_isBudgetWeight = false;
				UIntRange			m_range;
			};

			struct Node
			{
				Node()
				{

				}

				Node(
					const SourceNode*	aSource,
					bool				aWeighted)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "weapon_cooldown")
							m_weaponCooldown = aChild->GetUInt32();
						else if (aChild->m_name == "weapon_damage")
							m_weaponDamage = UIntRange(aChild);
						else if (aChild->m_name == "name")
							m_name = aChild->GetString();
						else if (aChild->m_name == "suffix")
							m_suffix = aChild->GetString();
						else if(aChild->m_name == "weighted_child")
							m_weightedChildren.push_back(std::make_unique<Node>(aChild, true));
						else if (aChild->m_name == "random_child")
							m_randomChildren.push_back(std::make_unique<Node>(aChild, false));
						else if (aChild->m_name == "child_chance" && !aWeighted)
							m_chance = aChild->GetProbability();
						else if (aChild->m_name == "child_weight" && aWeighted)
							m_weight = aChild->GetUInt32();
						else if (aChild->m_tag == "stat")
							m_addedStats.push_back(AddedStat(aChild, false));
						else if (aChild->m_tag == "stat_weight")
							m_addedStats.push_back(AddedStat(aChild, true));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});

					for(std::unique_ptr<Node>& child : m_weightedChildren)
						m_totalChildWeight += child->m_weight;

					TP_VERIFY((m_totalChildWeight == 0 && m_weightedChildren.size() == 0) || (m_totalChildWeight > 0 && m_weightedChildren.size() > 0),
						aSource->m_debugInfo, "Invalid children.");
				}

				void 
				ToStream(
					IWriter*			aStream) const
				{
					aStream->WriteObjects(m_addedStats);
					aStream->WriteOptionalObject(m_weaponDamage);
					aStream->WriteUInt(m_weaponCooldown);
					aStream->WriteString(m_name);
					aStream->WriteString(m_suffix);
					aStream->WriteInt(m_budgetBias);
					aStream->WriteUInt(m_chance);
					aStream->WriteUInt(m_weight);
					aStream->WriteObjectPointers(m_weightedChildren);
					aStream->WriteObjectPointers(m_randomChildren);
				}

				bool
				FromStream(
					IReader*			aStream)
				{
					if (!aStream->ReadObjects(m_addedStats))
						return false;
					if (!aStream->ReadOptionalObject(m_weaponDamage))
						return false;
					if (!aStream->ReadUInt(m_weaponCooldown))
						return false;
					if (!aStream->ReadString(m_name))
						return false;
					if (!aStream->ReadString(m_suffix))
						return false;
					if (!aStream->ReadInt(m_budgetBias))
						return false;
					if (!aStream->ReadUInt(m_chance))
						return false;
					if (!aStream->ReadUInt(m_weight))
						return false;
					if (!aStream->ReadObjectPointers(m_weightedChildren))
						return false;
					if (!aStream->ReadObjectPointers(m_randomChildren))
						return false;

					for (std::unique_ptr<Node>& child : m_weightedChildren)
						m_totalChildWeight += child->m_weight;

					return true;
				}

				// Public data
				std::vector<AddedStat>				m_addedStats;
				std::optional<UIntRange>			m_weaponDamage;
				uint32_t							m_weaponCooldown = 0;
				std::string							m_name;
				std::string							m_suffix;
				uint32_t							m_chance = 0;
				uint32_t							m_weight = 1;				
				uint32_t							m_totalChildWeight = 0;
				int32_t								m_budgetBias = 0;
				std::vector<std::unique_ptr<Node>>	m_weightedChildren;
				std::vector<std::unique_ptr<Node>>	m_randomChildren;
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
					if(aChild->m_name == "equipment_slots")
					{
						aChild->ForEachChild([&](
							const SourceNode* aEquipmentSlot)
						{
							EquipmentSlot::Id id = EquipmentSlot::StringToId(aEquipmentSlot->GetIdentifier());
							TP_VERIFY(id != EquipmentSlot::INVALID_ID, aEquipmentSlot->m_debugInfo, "'%s' is not a valid equipment slot.", aEquipmentSlot->GetIdentifier());
							m_equipmentSlots.push_back(id);
						});	
					}
					else if(aChild->m_name == "loot_groups")
					{
						aChild->GetIdArray(DataType::ID_LOOT_GROUP, m_lootGroups);
					}
					else if(aChild->m_name == "root")
					{
						m_root = std::make_unique<Node>(aChild, false);
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
					else if(aChild->m_name == "type")
					{
						m_itemType = ItemType::StringToId(aChild->GetIdentifier());
						TP_VERIFY(m_itemType != ItemType::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid item type.", aChild->GetIdentifier());
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});

				if(m_itemLevel == 0)
					m_itemLevel = m_requiredLevel;
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteUInts(m_equipmentSlots);
				aStream->WriteUInts(m_lootGroups);
				aStream->WriteOptionalObjectPointer(m_root);
				aStream->WriteUInt(m_stackSize);
				aStream->WriteUInt(m_useAbilityId);
				aStream->WritePOD(m_itemType);
				m_levelRange.ToStream(aStream);
				aStream->WriteUInt(m_requiredLevel);
				aStream->WriteUInt(m_itemLevel);
				aStream->WriteUInt(m_iconSpriteId);
				aStream->WriteUInt(m_cost);
				aStream->WritePOD(m_rarity);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if (!aStream->ReadUInts(m_equipmentSlots))
					return false;
				if (!aStream->ReadUInts(m_lootGroups))
					return false;
				if(!aStream->ReadOptionalObjectPointer(m_root))
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
				return true;
			}

			// Public data
			std::vector<uint32_t>		m_equipmentSlots;
			std::vector<uint32_t>		m_lootGroups;
			std::unique_ptr<Node>		m_root;
			uint32_t					m_stackSize = 1;
			uint32_t					m_useAbilityId = 0;
			ItemType::Id				m_itemType = ItemType::ID_NONE;
			UIntRange					m_levelRange;
			uint32_t					m_requiredLevel = 1;
			uint32_t					m_itemLevel = 0;
			uint32_t					m_iconSpriteId = 0;
			uint32_t					m_cost = 0;
			Rarity::Id					m_rarity = Rarity::ID_COMMON;
		};

	}

}