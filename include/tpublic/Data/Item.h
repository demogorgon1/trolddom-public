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
				FLAG_KILL_CONTRIBUTION_LOOT			= 0x00000008,
				FLAG_QUEST_REWARD					= 0x00000010,
				FLAG_DUNGEON_LOOT					= 0x00000020,
				FLAG_FISHING_ROD					= 0x00000040,
				FLAG_STARTS_QUEST					= 0x00000080,
				FLAG_HEROIC							= 0x00000100,
				FLAG_VARIANT						= 0x00000200
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
					else if (strcmp(identifier, "quest_reward") == 0)
						flags |= FLAG_QUEST_REWARD;
					else if (strcmp(identifier, "dungeon_loot") == 0)
						flags |= FLAG_DUNGEON_LOOT;
					else if (strcmp(identifier, "fishing_rod") == 0)
						flags |= FLAG_FISHING_ROD;
					else if (strcmp(identifier, "starts_quest") == 0)
						flags |= FLAG_STARTS_QUEST;
					else if (strcmp(identifier, "heroic") == 0)
						flags |= FLAG_HEROIC;
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
				Stat::Id				m_id = Stat::Id(0);
				bool					m_isBudgetWeight = false;
				uint32_t				m_value = 0;
			};

			struct Zone
			{
				Zone()
				{

				}

				Zone(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "map")
							m_mapId = aChild->GetId(DataType::ID_MAP);
						else if (aChild->m_name == "zone")
							m_zoneId = aChild->GetId(DataType::ID_ZONE);
						else if(aChild->m_name == "loot_groups")
							aChild->GetIdArray(DataType::ID_ENTITY, m_lootGroupIds);
						else 
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void 
				ToStream(
					IWriter*			aStream) const
				{
					aStream->WriteUInt(m_mapId);
					aStream->WriteUInt(m_zoneId);
					aStream->WriteUInts(m_lootGroupIds);
				}

				bool
				FromStream(
					IReader*			aStream)
				{
					if (!aStream->ReadUInt(m_mapId))
						return false;
					if (!aStream->ReadUInt(m_zoneId))
						return false;
					if(!aStream->ReadUInts(m_lootGroupIds))
						return false;
					return true;
				}

				void
				AddUniqueLootGroupId(
					uint32_t			aLootGroupId)
				{
					for(uint32_t t : m_lootGroupIds)
					{
						if(t == aLootGroupId)
							return;
					}
					m_lootGroupIds.push_back(aLootGroupId);
				}

				// Public data
				uint32_t				m_mapId = 0;
				uint32_t				m_zoneId = 0;
				std::vector<uint32_t>	m_lootGroupIds;
			};

			struct Oracle
			{
				Oracle()
				{

				}

				Oracle(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "text" && aChild->IsArrayType(SourceNode::TYPE_STRING))
							aChild->GetStringArray(m_text);
						else if (aChild->m_name == "text")
							m_text.push_back(aChild->GetString());
						else if(aChild->m_name == "zones")
							aChild->GetObjectArray(m_zones);
						else 
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}


				void 
				ToStream(
					IWriter*			aStream) const
				{
					aStream->WriteStrings(m_text);
					aStream->WriteObjects(m_zones);
				}

				bool
				FromStream(
					IReader*			aStream)
				{
					if (!aStream->ReadStrings(m_text))
						return false;
					if(!aStream->ReadObjects(m_zones))
						return false;
					return true;
				}

				Zone*
				GetOrCreateZone(
					uint32_t			aMapId,
					uint32_t			aZoneId)
				{
					for(Zone& t : m_zones)
					{
						if(t.m_zoneId == aZoneId && t.m_mapId == aMapId)
							return &t;
					}

					Zone n;
					n.m_mapId = aMapId;
					n.m_zoneId = aZoneId;
					m_zones.push_back(n);
					return &m_zones[m_zones.size() - 1];
				}

				// Public data
				std::vector<std::string>	m_text;
				std::vector<Zone>			m_zones;
			};

			struct Variant
			{
				Variant()
				{

				}

				Variant(
					const SourceNode*	aSource)
				{
					m_suffix = aSource->m_name;

					m_data.reset(new Item());
					m_data->FromSource(aSource);
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteString(m_suffix);
					aWriter->WriteObjectPointer(m_data);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!aReader->ReadString(m_suffix))
						return false;
					if(!aReader->ReadObjectPointer(m_data))
						return false;
					return true;
				}

				// Public data
				std::string					m_suffix;
				std::unique_ptr<Item>		m_data;
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

			void	ReadSource(
						const SourceNode*	aSource);
			void	InitVariant(
						const Item*			aVariant);

			// Helpers
			bool	IsUnique() const { return m_flags & FLAG_UNIQUE; }
			bool	IsNotSellable() const { return m_flags & FLAG_NOT_SELLABLE; }
			bool	IsVendor() const { return m_flags & FLAG_VENDOR; }
			bool	IsKillContributionLoot() const { return m_flags & FLAG_KILL_CONTRIBUTION_LOOT; }
			bool	IsQuestReward() const { return m_flags & FLAG_QUEST_REWARD; }
			bool	IsDefined() const { return m_iconSpriteId != 0 && !m_string.empty(); }
			bool	DoesStartQuest() const { return m_flags & FLAG_STARTS_QUEST; }
			bool	IsHeroic() const { return m_flags & FLAG_HEROIC; }

			// Base implementation
			void
			FromSource(
				const SourceNode*			aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
						ReadSource(aChild);
				});

				if(m_itemLevel == 0)
					m_itemLevel = m_requiredLevel;

				if(m_itemType == ItemType::ID_NONE)
				{
					if(m_useAbilityId != 0)
						m_itemType = ItemType::ID_CONSUMABLE;
					else if(IsEquippableInSlot(EquipmentSlot::ID_FINGER_1) || IsEquippableInSlot(EquipmentSlot::ID_FINGER_2) || IsEquippableInSlot(EquipmentSlot::ID_NECK))
						m_itemType = ItemType::ID_JEWELRY;
					else if (IsEquippableInSlot(EquipmentSlot::ID_BACK))
						m_itemType = ItemType::ID_ARMOR_CLOTH;
					else
						m_itemType = ItemType::ID_MISCELLANEOUS;
				}

				for(uint32_t equipmentSlot : m_equipmentSlots)
				{
					const EquipmentSlot::Info* equipmentSlotInfo = EquipmentSlot::GetInfo((EquipmentSlot::Id)equipmentSlot);
					if(equipmentSlotInfo->m_itemTypes.size() > 0)
					{
						bool validItemType = Helpers::FindItem(equipmentSlotInfo->m_itemTypes, m_itemType) != SIZE_MAX;
						TP_VERIFY(validItemType, aSource->m_debugInfo, "'%s' does not have a valid item type.", aSource->m_name.c_str());
					}
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
				aStream->WriteOptionalObjectPointer(m_oracle);
				aStream->WriteUInt(m_empower);
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
				if(!aStream->ReadOptionalObjectPointer(m_oracle))
					return false;
				if (!aStream->ReadUInt(m_empower))
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
				aOut["quest_reward"] = IsQuestReward() ? "1" : "0";

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

			void		PrepareRuntime( 
							uint8_t					aRuntime,
							const Manifest*			aManifest) override;

			// Public data
			std::vector<uint32_t>					m_equipmentSlots;
			std::vector<uint32_t>					m_lootGroups;
			uint32_t								m_stackSize = 1;
			uint32_t								m_useAbilityId = 0;
			ItemType::Id							m_itemType = ItemType::ID_NONE;
			UIntRange								m_levelRange;
			uint32_t								m_requiredLevel = 0;
			uint32_t								m_itemLevel = 0;
			uint32_t								m_iconSpriteId = 0;
			uint32_t								m_cost = 0;
			Rarity::Id								m_rarity = Rarity::ID_COMMON;
			std::vector<AddedStat>					m_addedStats;
			std::optional<UIntRange>				m_weaponDamage;
			uint32_t								m_weaponCooldown = 0;
			std::string								m_string;
			std::string								m_flavor;
			int32_t									m_budgetBias = 0;
			ItemBinding::Id							m_itemBinding = ItemBinding::ID_NEVER;
			SoundEffect::Collection					m_soundEffects;
			uint32_t								m_flags = 0;
			uint32_t								m_bagSlots = 0;
			float									m_valueMultiplier = 1.0f;
			std::optional<ArmorStyle::Visual>		m_armorStyleVisual;
			uint32_t								m_questId = 0;
			uint32_t								m_auraId = 0;
			uint32_t								m_tokenCost = 0;
			std::unique_ptr<Oracle>					m_oracle;
			uint32_t								m_empower = 0;
			std::vector<std::unique_ptr<Variant>>	m_variants;

			// Not serialized
			std::string								m_lcString;
		};

	}

}