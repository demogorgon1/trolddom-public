#pragma once

#include "../ArmorStyle.h"
#include "../DataBase.h"
#include "../EquipmentSlot.h"
#include "../Helpers.h"
#include "../ItemBinding.h"
#include "../ItemType.h"
#include "../Rarity.h"
#include "../SoundEffect.h"
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

			enum Flag : uint32_t
			{
				FLAG_UNIQUE							= 0x00000001,
				FLAG_NOT_SELLABLE					= 0x00000002,
				FLAG_VENDOR							= 0x00000004,
				FLAG_KILL_CONTRIBUTION_LOOT			= 0x00000008
			};

			static inline uint32_t
			SourceToFlags(
				const SourceNode*			aSource)
			{
				uint32_t flags = 0;
				aSource->GetArray()->ForEachChild([&](
					const SourceNode*		aChild)
				{
					const char* identifier = aChild->GetIdentifier();
					if (strcmp(identifier, "unique") == 0)
						flags |= FLAG_UNIQUE;
					else if (strcmp(identifier, "not_sellable") == 0)
						flags |= FLAG_NOT_SELLABLE;
					else if (strcmp(identifier, "vendor") == 0)
						flags |= FLAG_VENDOR;
					else if (strcmp(identifier, "kill_contribution_loot") == 0)
						flags |= FLAG_KILL_CONTRIBUTION_LOOT;
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item flag.", identifier);
				});
				return flags;
			}

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

			const ArmorStyle::Visual&
			GetArmorStyleVisual() const
			{
				if(m_armorStyleVisual)
					return m_armorStyleVisual.value();

				// FIXME: this is a bit lazy... (don't have some weird race condition during init)
				static ArmorStyle::Visual defaultArmorStyleVisual;
				return defaultArmorStyleVisual;
			}

			// Helpers
			bool	IsUnique() const { return m_flags & FLAG_UNIQUE; }
			bool	IsNotSellable() const { return m_flags & FLAG_NOT_SELLABLE; }
			bool	IsVendor() const { return m_flags & FLAG_VENDOR; }
			bool	IsKillContributionLoot() const { return m_flags & FLAG_KILL_CONTRIBUTION_LOOT; }

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
						else if (aChild->m_name == "use_ability")
						{
							m_useAbilityId = aChild->GetId(DataType::ID_ABILITY);
						}
						else if (aChild->m_name == "icon")
						{
							m_iconSpriteId = aChild->GetId(DataType::ID_SPRITE);
						}
						else if (aChild->m_name == "quest")
						{
							m_questId = aChild->GetId(DataType::ID_QUEST);
						}
						else if (aChild->m_name == "aura")
						{
							m_auraId = aChild->GetId(DataType::ID_AURA);
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
						else if (aChild->m_name == "bag_slots")
						{
							m_bagSlots = aChild->GetUInt32();
						}
						else if (aChild->m_name == "token_cost")
						{
							m_tokenCost = aChild->GetUInt32();
						}
						else if (aChild->m_name == "weapon_damage")
						{
							m_weaponDamage = UIntRange(aChild);
						}
						else if (aChild->m_name == "string")
						{
							m_string = aChild->GetString();
						}
						else if (aChild->m_name == "flags")
						{
							m_flags = SourceToFlags(aChild);
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
						else if(aChild->m_name == "sound_effects")
						{
							m_soundEffects.FromSource(aChild);
						}
						else if (aChild->m_name == "value_multiplier")
						{
							m_valueMultiplier = aChild->GetFloat();
						}
						else if(aChild->m_name == "armor_style_visual")
						{
							m_armorStyleVisual = ArmorStyle::Visual(aChild);
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					}
				});

				if(m_itemLevel == 0)
					m_itemLevel = m_requiredLevel;

				if(m_itemType == ItemType::ID_NONE)
				{
					if(m_useAbilityId != 0)
						m_itemType = ItemType::ID_CONSUMABLE;
					else
						m_itemType = ItemType::ID_MISCELLANEOUS;
				}
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
				aStream->WriteObjects(m_addedStats);
				aStream->WriteOptionalObject(m_weaponDamage);
				aStream->WriteUInt(m_weaponCooldown);
				aStream->WriteUInt(m_bagSlots);
				aStream->WriteString(m_string);
				aStream->WriteString(m_flavor);
				aStream->WriteInt(m_budgetBias);
				aStream->WritePOD(m_itemBinding);
				m_soundEffects.ToStream(aStream);
				aStream->WritePOD(m_flags);
				aStream->WriteFloat(m_valueMultiplier);
				aStream->WriteOptionalObject(m_armorStyleVisual);
				aStream->WriteUInt(m_questId);
				aStream->WriteUInt(m_auraId);
				aStream->WriteUInt(m_tokenCost);
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
				if (!aStream->ReadObjects(m_addedStats))
					return false;
				if (!aStream->ReadOptionalObject(m_weaponDamage))
					return false;
				if (!aStream->ReadUInt(m_weaponCooldown))
					return false;
				if (!aStream->ReadUInt(m_bagSlots))
					return false;
				if (!aStream->ReadString(m_string))
					return false;
				if (!aStream->ReadString(m_flavor))
					return false;
				if (!aStream->ReadInt(m_budgetBias))
					return false;
				if (!aStream->ReadPOD(m_itemBinding))
					return false;
				if(!m_soundEffects.FromStream(aStream))
					return false;
				if (!aStream->ReadPOD(m_flags))
					return false;
				if (!aStream->ReadFloat(m_valueMultiplier))
					return false;
				if(!aStream->ReadOptionalObject(m_armorStyleVisual))
					return false;
				if (!aStream->ReadUInt(m_questId))
					return false;
				if (!aStream->ReadUInt(m_auraId))
					return false;
				if (!aStream->ReadUInt(m_tokenCost))
					return false;

				m_lcString = m_string;
				Helpers::MakeLowerCase(m_lcString);

				return true;
			}

			void
			ToPropertyTable(
				PropertyTable&			aOut) const override
			{
				ToPropertyTableBase(aOut);
				aOut["string"] = m_string;
				aOut["item_level"] = Helpers::Format("%u", m_itemLevel).c_str();
				aOut["required_level"] = Helpers::Format("%u", m_requiredLevel).c_str();
				aOut["rarity"] = Rarity::GetInfo(m_rarity)->m_name;
				aOut["item_type"] = ItemType::GetInfo(m_itemType)->m_name;

				std::string slots;
				for(uint32_t equipmentSlotId : m_equipmentSlots)
				{
					if(!slots.empty())
						slots += ",";
					slots += EquipmentSlot::GetInfo((EquipmentSlot::Id)equipmentSlotId)->m_name;
				}
				if(!slots.empty())
					aOut["slots"] = slots;

				if(m_weaponCooldown != 0)
					aOut["weapon_cooldown"] = Helpers::Format("%u", m_weaponCooldown).c_str();

				if (m_weaponDamage)
					aOut["weapon_damage"] = Helpers::Format("%u-%u", m_weaponDamage->m_min, m_weaponDamage->m_max).c_str();
			}

			// Public data
			std::vector<uint32_t>				m_equipmentSlots;
			std::vector<uint32_t>				m_lootGroups;
			uint32_t							m_stackSize = 1;
			uint32_t							m_useAbilityId = 0;
			ItemType::Id						m_itemType = ItemType::ID_NONE;
			UIntRange							m_levelRange;
			uint32_t							m_requiredLevel = 0;
			uint32_t							m_itemLevel = 0;
			uint32_t							m_iconSpriteId = 0;
			uint32_t							m_cost = 0;
			Rarity::Id							m_rarity = Rarity::ID_COMMON;
			std::vector<AddedStat>				m_addedStats;
			std::optional<UIntRange>			m_weaponDamage;
			uint32_t							m_weaponCooldown = 0;
			std::string							m_string;
			std::string							m_flavor;
			int32_t								m_budgetBias = 0;
			ItemBinding::Id						m_itemBinding = ItemBinding::ID_NEVER;
			SoundEffect::Collection				m_soundEffects;
			uint32_t							m_flags = 0;
			uint32_t							m_bagSlots = 0;
			float								m_valueMultiplier = 1.0f;
			std::optional<ArmorStyle::Visual>	m_armorStyleVisual;
			uint32_t							m_questId = 0;
			uint32_t							m_auraId = 0;
			uint32_t							m_tokenCost = 0;

			// Not serialized
			std::string							m_lcString;
		};

	}

}