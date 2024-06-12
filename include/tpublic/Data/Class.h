#pragma once

#include "../ActionBar.h"
#include "../DataBase.h"
#include "../EquipmentSlot.h"
#include "../ItemType.h"
#include "../Resource.h"
#include "../Stat.h"
#include "../System.h"

namespace tpublic
{

	namespace Data
	{

		struct Class
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_CLASS;
			static const bool TAGGED = true;

			struct Color
			{
				uint8_t					m_r = 0;
				uint8_t					m_g = 0;
				uint8_t					m_b = 0;
			};

			struct StartEquipment
			{
				StartEquipment()
				{

				}

				StartEquipment(
					const SourceNode*		aSource)
				{
					m_equipmentSlotId = EquipmentSlot::StringToId(aSource->m_name.c_str());
					TP_VERIFY(m_equipmentSlotId != 0, aSource->m_debugInfo, "'%s' not a valid equipment slot.", aSource->m_name.c_str());
					m_itemId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ITEM, aSource->GetIdentifier());
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_equipmentSlotId);
					aStream->WriteUInt(m_itemId);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if (!aStream->ReadUInt(m_equipmentSlotId))
						return false;
					if (!aStream->ReadUInt(m_itemId))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_equipmentSlotId = 0;
				uint32_t				m_itemId = 0;				
			};

			struct StartMap
			{
				StartMap()
				{

				}

				StartMap(
					const SourceNode*		aSource)
				{
					m_mapId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP, aSource->m_name.c_str());
					aSource->GetIdArray(DataType::ID_MAP_PLAYER_SPAWN, m_mapPlayerSpawnIds);					
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_mapId);
					aStream->WriteUInts(m_mapPlayerSpawnIds);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if (!aStream->ReadUInt(m_mapId))
						return false;
					if (!aStream->ReadUInts(m_mapPlayerSpawnIds))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_mapId = 0;
				std::vector<uint32_t>	m_mapPlayerSpawnIds;				
			};

			struct LevelProgressionLevelResourceUpdate
			{	
				LevelProgressionLevelResourceUpdate()
				{

				}

				LevelProgressionLevelResourceUpdate(
					const SourceNode*	aNode)
				{
					m_resourceId = Resource::StringToId(aNode->m_name.c_str());
					TP_VERIFY(m_resourceId != 0, aNode->m_debugInfo, "'%s' not a valid resource.", aNode->m_name.c_str());

					aNode->ForEachChild([&](
						const SourceNode* aMember)
					{						
						if (aMember->m_name == "add_max")
							m_addMax = aMember->GetUInt32();
						else
							TP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
					});
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_resourceId);
					aStream->WriteUInt(m_addMax);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if (!aStream->ReadUInt(m_resourceId))
						return false;
					if (!aStream->ReadUInt(m_addMax))
						return false;
					return true;
				}
				
				// Public data
				uint32_t											m_resourceId = 0;
				uint32_t											m_addMax = 0;
			};

			struct LevelProgressionLevel
			{
				LevelProgressionLevel()
				{

				}

				LevelProgressionLevel(
					const SourceNode*	aNode)
				{
					aNode->ForEachChild([&](
						const SourceNode* aMember)
					{						
						if (aMember->m_name == "level")
						{
							m_level = aMember->GetUInt32();
						}
						else if (aMember->m_name == "unlock_abilities")
						{
							aMember->GetIdArray(DataType::ID_ABILITY, m_unlockAbilities);
						}
						else if (aMember->m_name == "stats")
						{
							m_stats.FromSource(aMember);
						}
						else if(aMember->m_tag == "resource")
						{
							m_resourceUpdates.push_back(LevelProgressionLevelResourceUpdate(aMember));
						}
						else if(aMember->m_name == "add_unarmed_weapon_damage")
						{
							TP_VERIFY(aMember->m_type == SourceNode::TYPE_ARRAY && aMember->m_children.size() == 2, aMember->m_debugInfo, "Not a valid range.");
							m_addUnarmedWeaponDamageMin = aMember->m_children[0]->GetUInt32();
							m_addUnarmedWeaponDamageMax = aMember->m_children[1]->GetUInt32();
						}
						else
						{
							TP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
						}
					});
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_level);
					aStream->WriteUInts(m_unlockAbilities);
					m_stats.ToStream(aStream);
					aStream->WriteObjects(m_resourceUpdates);
					aStream->WriteUInt(m_addUnarmedWeaponDamageMin);
					aStream->WriteUInt(m_addUnarmedWeaponDamageMax);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadUInt(m_level))
						return false;
					if (!aStream->ReadUInts(m_unlockAbilities))
						return false;
					if(!m_stats.FromStream(aStream))
						return false;
					if(!aStream->ReadObjects(m_resourceUpdates))
						return false;
					if (!aStream->ReadUInt(m_addUnarmedWeaponDamageMin))
						return false;
					if (!aStream->ReadUInt(m_addUnarmedWeaponDamageMax))
						return false;
					return true;
				}

				// Public data
				uint32_t											m_level = 1;
				std::vector<uint32_t>								m_unlockAbilities;
				Stat::Collection									m_stats;
				std::vector<LevelProgressionLevelResourceUpdate>	m_resourceUpdates;
				uint32_t											m_addUnarmedWeaponDamageMin = 0;
				uint32_t											m_addUnarmedWeaponDamageMax = 0;
			};

			struct LevelProgression
			{
				LevelProgression()
				{

				}

				LevelProgression(
					const SourceNode* aNode)
				{
					aNode->ForEachChild([&](
						const SourceNode* aArrayItem)
					{						
						m_levels.push_back(std::make_unique<LevelProgressionLevel>(aArrayItem->GetObject()));
					});
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteObjectPointers(m_levels);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadObjectPointers(m_levels))
						return false;
					return true;
				}

				const LevelProgressionLevel*
				GetLevel(
					uint32_t				aLevel) const
				{
					for(const std::unique_ptr<LevelProgressionLevel>& level : m_levels)
					{
						if(level->m_level == aLevel)
							return level.get();
					}
					return NULL;
				}

				// Public data
				std::vector<std::unique_ptr<LevelProgressionLevel>>	m_levels;
			};

			struct StatsConversionEntry
			{
				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteFloat(m_value);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadFloat(m_value))
						return false;
					return true;
				}

				void
				FromSource(
					const SourceNode*		aSource)
				{
					TP_VERIFY(aSource->m_type == SourceNode::TYPE_ARRAY && aSource->m_children.size() == 2, aSource->m_debugInfo, "Not a valid stats conversion.");
					m_value = (float)aSource->m_children[0]->GetUInt32() / (float)aSource->m_children[1]->GetUInt32();
				}

				void
				Combine(
					const StatsConversionEntry&	aOther)
				{
					m_value += aOther.m_value;
				}

				bool
				IsSet() const
				{
					return m_value != 0;
				}

				// Public data
				float						m_value = 0.0f;
			};

			struct StatsConversion
			{
				void
				FromSource(
					const SourceNode* aNode)
				{
					aNode->ForEachChild([&](
						const SourceNode* aItem)
					{				
						if (aItem->m_name == "str_to_dps")
							m_strToDps.FromSource(aItem);
						else if (aItem->m_name == "str_weapon_damage_min")
							m_strWeaponDamageMin.FromSource(aItem);
						else if (aItem->m_name == "str_weapon_damage_max")
							m_strWeaponDamageMax.FromSource(aItem);
						else if (aItem->m_name == "dex_to_phys_crit")
							m_dexToPhysCrit.FromSource(aItem);
						else if (aItem->m_name == "dex_to_dodge")
							m_dexToDodge.FromSource(aItem);
						else if (aItem->m_name == "dex_to_parry")
							m_dexToParry.FromSource(aItem);
						else if (aItem->m_name == "str_to_parry")
							m_strToParry.FromSource(aItem);
						else if (aItem->m_name == "wis_to_mag_crit")
							m_wisToMagCrit.FromSource(aItem);
						else if (aItem->m_name == "con_to_health")
							m_conToHealth.FromSource(aItem);
						else if (aItem->m_name == "wis_to_mana")
							m_wisToMana.FromSource(aItem);
						else if (aItem->m_name == "spi_to_health_regen")
							m_spiToHealthRegen.FromSource(aItem);
						else if (aItem->m_name == "spi_to_mana_regen_per_5_sec")
							m_spiToManaRegenPer5Sec.FromSource(aItem);
						else if (aItem->m_name == "str_to_block_value")
							m_strToBlockValue.FromSource(aItem);
						else
							TP_VERIFY(false, aItem->m_debugInfo, "'%s' not a valid item.", aItem->m_name.c_str());
					});
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					m_strToDps.ToStream(aStream);
					m_strWeaponDamageMin.ToStream(aStream);
					m_strWeaponDamageMax.ToStream(aStream);
					m_dexToPhysCrit.ToStream(aStream);
					m_dexToDodge.ToStream(aStream);
					m_dexToParry.ToStream(aStream);
					m_strToParry.ToStream(aStream);
					m_wisToMagCrit.ToStream(aStream);
					m_conToHealth.ToStream(aStream);
					m_wisToMana.ToStream(aStream);
					m_spiToHealthRegen.ToStream(aStream);
					m_spiToManaRegenPer5Sec.ToStream(aStream);
					m_strToBlockValue.ToStream(aStream);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if (!m_strToDps.FromStream(aStream))
						return false;
					if (!m_strWeaponDamageMin.FromStream(aStream))
						return false;
					if (!m_strWeaponDamageMax.FromStream(aStream))
						return false;
					if (!m_dexToPhysCrit.FromStream(aStream))
						return false;
					if (!m_dexToDodge.FromStream(aStream))
						return false;
					if (!m_dexToParry.FromStream(aStream))
						return false;
					if (!m_strToParry.FromStream(aStream))
						return false;
					if (!m_wisToMagCrit.FromStream(aStream))
						return false;
					if (!m_conToHealth.FromStream(aStream))
						return false;
					if (!m_wisToMana.FromStream(aStream))
						return false;
					if (!m_spiToHealthRegen.FromStream(aStream))
						return false;
					if (!m_spiToManaRegenPer5Sec.FromStream(aStream))
						return false;
					if (!m_strToBlockValue.FromStream(aStream))
						return false;
					return true;
				}

				// Public data
				StatsConversionEntry								m_strToDps;
				StatsConversionEntry								m_strWeaponDamageMin;
				StatsConversionEntry								m_strWeaponDamageMax;
				StatsConversionEntry								m_dexToPhysCrit;
				StatsConversionEntry								m_dexToDodge;
				StatsConversionEntry								m_dexToParry;
				StatsConversionEntry								m_strToParry;
				StatsConversionEntry								m_wisToMagCrit;
				StatsConversionEntry								m_conToHealth;
				StatsConversionEntry								m_wisToMana;
				StatsConversionEntry								m_spiToHealthRegen;
				StatsConversionEntry								m_spiToManaRegenPer5Sec;
				StatsConversionEntry								m_strToBlockValue;
			};

			void
			Verify() const
			{
				VerifyBase();

				TP_VERIFY(!m_displayName.empty(), m_debugInfo, "'%s' has no 'string'.", m_name.c_str());
			}

			bool
			CanUseItemType(
				ItemType::Id			aItemType) const
			{
				if(aItemType == ItemType::ID_NONE)
					return true;

				uint32_t i = (uint32_t)aItemType;
				return (m_itemTypesMask & (1 << i)) != 0;
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{
				aNode->ForEachChild([&](
					const SourceNode* aMember)
				{
					if(!FromSourceBase(aMember))
					{
						if (aMember->m_name == "string")
						{
							m_displayName = aMember->GetString();
						}
						else if (aMember->m_name == "description")
						{
							m_description = aMember->GetString();
						}
						else if (aMember->m_name == "sprite")
						{
							m_spriteId = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aMember->GetIdentifier());
						}
						else if (aMember->m_name == "sprite_dead")
						{
							m_spriteDeadId = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aMember->GetIdentifier());
						}
						else if (aMember->m_name == "color_1")
						{
							const SourceNode* components = aMember->GetArray();
							TP_VERIFY(components->m_children.size() == 3, aMember->m_debugInfo, "'%s' is not a valid color.", aMember->m_name.c_str());
							m_color1.m_r = components->m_children[0]->GetUInt8();
							m_color1.m_g = components->m_children[1]->GetUInt8();
							m_color1.m_b = components->m_children[2]->GetUInt8();
						}
						else if (aMember->m_name == "color_2")
						{
							const SourceNode* components = aMember->GetArray();
							TP_VERIFY(components->m_children.size() == 3, aMember->m_debugInfo, "'%s' is not a valid color.", aMember->m_name.c_str());
							m_color2.m_r = components->m_children[0]->GetUInt8();
							m_color2.m_g = components->m_children[1]->GetUInt8();
							m_color2.m_b = components->m_children[2]->GetUInt8();
						}
						else if (aMember->m_name == "default_attack")
						{
							m_defaultAttackAbilityId = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY, aMember->GetIdentifier());
						}
						else if (aMember->m_name == "default_action_bar")
						{
							aMember->GetIdArray(DataType::ID_ABILITY, m_defaultActionBar.m_slots);
						}
						else if (aMember->m_name == "level_progression")
						{
							m_levelProgression = std::make_unique<LevelProgression>(aMember->GetArray());
						}
						else if (aMember->m_tag == "start_equipment")
						{
							m_startEquipment.push_back(StartEquipment(aMember));
						}
						else if (aMember->m_tag == "start_map")
						{
							m_startMaps.push_back(std::make_unique<StartMap>(aMember));
						}
						else if (aMember->m_name == "stats_conversion")
						{
							m_statsConversion.FromSource(aMember);
						}
						else if (aMember->m_name == "talent_trees")
						{
							aMember->GetIdArray(DataType::ID_TALENT_TREE, m_talentTrees);
						}
						else if (aMember->m_name == "start_inventory")
						{
							aMember->GetIdArray(DataType::ID_ITEM, m_startInventory);
						}
						else if (aMember->m_name == "sprites_walk")
						{
							aMember->GetIdArray(DataType::ID_SPRITE, m_walkSpriteIds);
						}
						else if (aMember->m_name == "item_types")
						{
							std::vector<ItemType::Id> itemTypes;
							aMember->GetIdArrayWithLookup<ItemType::Id, ItemType::INVALID_ID>(itemTypes, [&](
								const char* aIdentifier) -> ItemType::Id
							{
								return ItemType::StringToId(aIdentifier);
							});
							for (ItemType::Id itemType : itemTypes)
							{
								uint32_t t = (uint32_t)itemType;
								static_assert((uint32_t)ItemType::NUM_IDS <= 32);
								m_itemTypesMask |= (1 << t);
							}
						}
						else if (aMember->m_name == "systems")
						{
							aMember->GetIdArrayWithLookup<uint32_t, System::INVALID_ID>(m_systems, [&](
								const char* aIdentifier) -> uint32_t
							{
								return (uint32_t)System::StringToId(aIdentifier);
							});
						}
						else
						{
							TP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
						}
					}
				});
			}

			void	
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteString(m_displayName);
				aStream->WriteString(m_description);
				aStream->WriteOptionalObjectPointer(m_levelProgression);
				aStream->WriteUInt(m_spriteId);
				aStream->WriteUInt(m_spriteDeadId);
				aStream->WriteUInt(m_defaultAttackAbilityId);
				aStream->WriteObjects(m_startEquipment);
				m_defaultActionBar.ToStream(aStream);
				aStream->WritePOD(m_color1);
				aStream->WritePOD(m_color2);
				aStream->WriteObjectPointers(m_startMaps);
				m_statsConversion.ToStream(aStream);
				aStream->WriteUInts(m_talentTrees);
				aStream->WritePOD(m_itemTypesMask);
				aStream->WriteUInts(m_systems);
				aStream->WriteUInts(m_startInventory);
				aStream->WriteUInts(m_walkSpriteIds);
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadString(m_displayName))
					return false;
				if (!aStream->ReadString(m_description))
					return false;
				if(!aStream->ReadOptionalObjectPointer(m_levelProgression))
					return false;
				if (!aStream->ReadUInt(m_spriteId))
					return false;
				if (!aStream->ReadUInt(m_spriteDeadId))
					return false;
				if (!aStream->ReadUInt(m_defaultAttackAbilityId))
					return false;
				if(!aStream->ReadObjects(m_startEquipment))
					return false;
				if(!m_defaultActionBar.FromStream(aStream))
					return false;
				if (!aStream->ReadPOD(m_color1))
					return false;
				if (!aStream->ReadPOD(m_color2))
					return false;
				if (!aStream->ReadObjectPointers(m_startMaps))
					return false;
				if(!m_statsConversion.FromStream(aStream))
					return false;
				if (!aStream->ReadUInts(m_talentTrees))
					return false;
				if (!aStream->ReadPOD(m_itemTypesMask))
					return false;
				if (!aStream->ReadUInts(m_systems))
					return false;
				if (!aStream->ReadUInts(m_startInventory))
					return false;
				if (!aStream->ReadUInts(m_walkSpriteIds))
					return false;
				return true;
			}

			// Public data
			std::string												m_displayName;
			std::string												m_description;
			uint32_t												m_spriteId = 0;
			uint32_t												m_spriteDeadId = 0;
			std::vector<uint32_t>									m_walkSpriteIds;
			Color													m_color1;
			Color													m_color2;
			uint32_t												m_defaultAttackAbilityId = 0;
			std::unique_ptr<LevelProgression>						m_levelProgression;
			std::vector<StartEquipment>								m_startEquipment;
			std::vector<uint32_t>									m_startInventory;
			ActionBar												m_defaultActionBar;
			std::vector<std::unique_ptr<StartMap>>					m_startMaps;
			StatsConversion											m_statsConversion;
			std::vector<uint32_t>									m_talentTrees;
			uint32_t												m_itemTypesMask = 0;
			std::vector<uint32_t>									m_systems;
		};

	}

}
