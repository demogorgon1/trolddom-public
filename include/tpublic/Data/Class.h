#pragma once

#include "../ActionBar.h"
#include "../ArmorStyle.h"
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

			struct GearOptimization
			{
				struct Entry
				{
					Entry()
					{
					}

					Entry(
						const SourceNode*	aSource)
					{
						m_statId = Stat::StringToId(aSource->m_name.c_str());
						TP_VERIFY(m_statId != Stat::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid stat.", aSource->m_name.c_str());
						m_weight = aSource->GetFloat();
					}

					void
					ToStream(
						IWriter*			aWriter) const
					{
						aWriter->WritePOD(m_statId);
						aWriter->WriteFloat(m_weight);
					}

					bool
					FromStream(
						IReader*			aReader)
					{
						if(!aReader->ReadPOD(m_statId))
							return false;
						else if(!aReader->ReadFloat(m_weight))
							return false;
						return true;
					}

					// Public data
					Stat::Id			m_statId = Stat::INVALID_ID;
					float				m_weight = 0.0f;
				};

				GearOptimization()
				{
					
				}

				GearOptimization(
					const SourceNode*		aSource)
				{
					m_name = aSource->m_name;

					if(aSource->m_annotation)
					{
						aSource->m_annotation->GetObject()->ForEachChild([&](
							const SourceNode* aChild)
						{
							if(aChild->m_name == "weapon_dps_weight")
								m_weaponDPSWeight = aChild->GetFloat();
							else
								TP_VERIFY(false, aChild->m_debugInfo, "'%s' not a valid item.", aChild->m_name.c_str());
						});
					}

					aSource->GetObject()->ForEachChild([&](
						const SourceNode*	aChild)
					{
						m_entries.push_back(Entry(aChild));
					});
				}

				void
				ToStream(
					IWriter*				aWriter) const
				{
					aWriter->WriteString(m_name);
					aWriter->WriteObjects(m_entries);
					aWriter->WriteFloat(m_weaponDPSWeight);
				}

				bool
				FromStream(
					IReader*				aReader)
				{
					if(!aReader->ReadString(m_name))
						return false;
					else if(!aReader->ReadObjects(m_entries))
						return false;
					else if(!aReader->ReadFloat(m_weaponDPSWeight))
						return false;
					return true;
				}

				// Public data
				std::string				m_name;
				std::vector<Entry>		m_entries;
				float					m_weaponDPSWeight = 0.0f;
			};

			struct Color
			{
				uint8_t					m_r = 0;
				uint8_t					m_g = 0;
				uint8_t					m_b = 0;
			};

			struct SpriteCollection
			{
				SpriteCollection()
				{

				}

				SpriteCollection(
					const SourceNode*		aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if (aChild->m_name == "sprite")
							m_spriteId = aChild->GetId(DataType::ID_SPRITE);
						else if (aChild->m_name == "sprite_dead")
							m_deadSpriteId = aChild->GetId(DataType::ID_SPRITE);
						else if (aChild->m_name == "sprite_mounted")
							m_mountedSpriteId = aChild->GetId(DataType::ID_SPRITE);
						else if (aChild->m_name == "sprites_walk")
							aChild->GetIdArray(DataType::ID_SPRITE, m_walkSpriteIds);
						else if(aChild->m_name == "mounted_offset")
							m_mountedOffset = aChild->GetVec2();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' not a valid item.", aChild->m_name.c_str());
					});
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_spriteId);
					aStream->WriteUInt(m_deadSpriteId);
					aStream->WriteUInt(m_mountedSpriteId);
					aStream->WriteUInts(m_walkSpriteIds);
					m_mountedOffset.ToStream(aStream);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if (!aStream->ReadUInt(m_spriteId))
						return false;
					if (!aStream->ReadUInt(m_deadSpriteId))
						return false;
					if (!aStream->ReadUInt(m_mountedSpriteId))
						return false;
					if (!aStream->ReadUInts(m_walkSpriteIds))
						return false;
					if(!m_mountedOffset.FromStream(aStream))
						return false;
					return true;
				}


				// Public data
				uint32_t				m_spriteId = 0;
				uint32_t				m_deadSpriteId = 0;
				uint32_t				m_mountedSpriteId = 0;
				std::vector<uint32_t>	m_walkSpriteIds;
				Vec2					m_mountedOffset;
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
					m_itemId = aSource->GetId(DataType::ID_ITEM);
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

			struct StartReputation
			{
				StartReputation()
				{

				}

				StartReputation(
					const SourceNode*		aSource)
				{
					m_factionId = aSource->m_sourceContext->m_persistentIdTable->GetId(aSource->m_debugInfo, DataType::ID_FACTION, aSource->m_name.c_str());
					m_reputation = aSource->GetInt32();
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_factionId);
					aStream->WriteInt(m_reputation);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if (!aStream->ReadUInt(m_factionId))
						return false;
					if (!aStream->ReadInt(m_reputation))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_factionId = 0;
				int32_t					m_reputation = 0;
			};

			struct StartMap
			{
				StartMap()
				{

				}

				StartMap(
					const SourceNode*		aSource)
				{
					m_mapId = aSource->m_sourceContext->m_persistentIdTable->GetId(aSource->m_debugInfo, DataType::ID_MAP, aSource->m_name.c_str());
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
						else if (aMember->m_name == "achievement")
						{
							m_achievementId = aMember->GetId(DataType::ID_ACHIEVEMENT);
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
					aStream->WriteUInt(m_achievementId);
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
					if (!aStream->ReadUInt(m_achievementId))
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
				uint32_t											m_achievementId = 0;
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
						if (aItem->m_name == "dex_to_ranged_dps")
							m_dexToRangedDps.FromSource(aItem);
						else if (aItem->m_name == "dex_ranged_damage_min")
							m_dexRangedDamageMin.FromSource(aItem);
						else if (aItem->m_name == "dex_ranged_damage_max")
							m_dexRangedDamageMax.FromSource(aItem);
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
						else if (aItem->m_name == "wis_to_spell_damage")
							m_wisToSpellDamage.FromSource(aItem);
						else if (aItem->m_name == "wis_to_healing")
							m_wisToHealing.FromSource(aItem);
						else if (aItem->m_name == "attack_power_to_dps")
							m_attackPowerToDps.FromSource(aItem);
						else if (aItem->m_name == "str_to_attack_power")
							m_strToAttackPower.FromSource(aItem);
						else if (aItem->m_name == "dex_to_attack_power")
							m_dexToAttackPower.FromSource(aItem);
						else
							TP_VERIFY(false, aItem->m_debugInfo, "'%s' not a valid item.", aItem->m_name.c_str());
					});
				}

				void	
				ToStream(
					IWriter*				aStream) const 
				{
					m_dexToRangedDps.ToStream(aStream);
					m_dexRangedDamageMin.ToStream(aStream);
					m_dexRangedDamageMax.ToStream(aStream);
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
					m_wisToSpellDamage.ToStream(aStream);
					m_wisToHealing.ToStream(aStream);
					m_attackPowerToDps.ToStream(aStream);
					m_strToAttackPower.ToStream(aStream);
					m_dexToAttackPower.ToStream(aStream);
				}
			
				bool	
				FromStream(
					IReader*				aStream) 
				{
					if (!m_dexToRangedDps.FromStream(aStream))
						return false;
					if (!m_dexRangedDamageMin.FromStream(aStream))
						return false;
					if (!m_dexRangedDamageMax.FromStream(aStream))
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
					if (!m_wisToSpellDamage.FromStream(aStream))
						return false;
					if (!m_wisToHealing.FromStream(aStream))
						return false;
					if (!m_attackPowerToDps.FromStream(aStream))
						return false;
					if (!m_strToAttackPower.FromStream(aStream))
						return false;
					if (!m_dexToAttackPower.FromStream(aStream))
						return false;
					return true;
				}

				// Public data
				StatsConversionEntry								m_dexToRangedDps;
				StatsConversionEntry								m_dexRangedDamageMin;
				StatsConversionEntry								m_dexRangedDamageMax;
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
				StatsConversionEntry								m_wisToSpellDamage;
				StatsConversionEntry								m_wisToHealing;
				StatsConversionEntry								m_attackPowerToDps;
				StatsConversionEntry								m_strToAttackPower;
				StatsConversionEntry								m_dexToAttackPower;
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

			uint32_t
			GetDefaultSpriteId() const
			{
				return m_armorStyles[m_defaultArmorStyleId].m_spriteId;
			}

			uint32_t
			GetStartEquipmentItemId(
				EquipmentSlot::Id		aEquipmentSlotId) const
			{
				for(const StartEquipment& t : m_startEquipment)
				{
					if(t.m_equipmentSlotId == aEquipmentSlotId)
						return t.m_itemId;
				}
				return 0;
			}

			const StartMap*
			GetStartMap(
				uint32_t				aMapId) const
			{
				for(const std::unique_ptr<StartMap>& t : m_startMaps)
				{
					if(t->m_mapId == aMapId)
						return t.get();
				}
				return NULL;
			}

			const GearOptimization*
			GetGearOptimization(
				const char*				aName) const
			{
				for(const std::unique_ptr<GearOptimization>& t : m_gearOptimizations)
				{
					if(t->m_name == aName)
						return t.get();
				}
				return NULL;
			}

			uint32_t
			GetAbilityReplacement(
				uint32_t				aAbilityId) const
			{
				std::unordered_map<uint32_t, uint32_t>::const_iterator i = m_replaceAbilityTable.find(aAbilityId);
				if(i == m_replaceAbilityTable.cend())
					return aAbilityId;
				return i->second;
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
							m_defaultAttackAbilityId = aMember->GetId(DataType::ID_ABILITY);
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
						else if(aMember->m_name == "default_armor_style")
						{
							m_defaultArmorStyleId = ArmorStyle::StringToId(aMember->GetIdentifier());
							TP_VERIFY(m_defaultArmorStyleId != ArmorStyle::INVALID_ID, aMember->m_debugInfo, "'%s' not a valid armor style.", aMember->GetIdentifier());
						}
						else if(aMember->m_name == "remains")
						{
							aMember->GetIdArray(DataType::ID_ENTITY, m_remains);
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
						else if(aMember->m_tag == "armor_style")
						{
							ArmorStyle::Id armorStyleId = ArmorStyle::StringToId(aMember->m_name.c_str());
							TP_VERIFY(armorStyleId != ArmorStyle::INVALID_ID, aMember->m_debugInfo, "'%s' not a valid armor style.", aMember->m_name.c_str());
							m_armorStyles[armorStyleId] = SpriteCollection(aMember);
						}
						else if (aMember->m_tag == "weapon_sprites")
						{
							ItemType::Id itemTypeId = ItemType::StringToId(aMember->m_name.c_str());
							TP_VERIFY(itemTypeId != ItemType::INVALID_ID, aMember->m_debugInfo, "'%s' not a valid item type.", aMember->m_name.c_str());
							bool weapon = ItemType::GetInfo(itemTypeId)->m_flags & ItemType::FLAG_WEAPON;
							bool shield = ItemType::GetInfo(itemTypeId)->m_flags & ItemType::FLAG_SHIELD;
							TP_VERIFY(weapon || shield, aMember->m_debugInfo, "'%s' is not a weapon or a shield.", aMember->m_name.c_str());
							m_weaponSprites[itemTypeId] = std::make_unique<SpriteCollection>(aMember);
						}
						else if(aMember->m_name == "armor_decoration")
						{
							m_armorDecoration = SpriteCollection(aMember);
						}
						else if(aMember->m_name == "unlocked_by_achievement")
						{
							m_unlockedByAchievementId = aMember->GetId(DataType::ID_ACHIEVEMENT);
						}
						else if(aMember->m_name == "restricted")
						{
							m_restricted = aMember->GetBool();
						}
						else if(aMember->m_name == "version")
						{
							m_version = aMember->GetUInt32();
						}
						else if(aMember->m_tag == "start_reputation")
						{
							m_startReputations.push_back(StartReputation(aMember));
						}
						else if(aMember->m_tag == "gear_optimization")
						{
							m_gearOptimizations.push_back(std::make_unique<GearOptimization>(aMember));
						}
						else if(aMember->m_tag == "replace_ability")
						{
							m_replaceAbilityTable[aMember->m_sourceContext->m_persistentIdTable->GetId(aMember->m_debugInfo, DataType::ID_ABILITY, aMember->m_name.c_str())] = aMember->GetId(DataType::ID_ABILITY);
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
				aStream->WriteUInts(m_remains);
				aStream->WritePOD(m_defaultArmorStyleId);
				m_armorDecoration.ToStream(aStream);
				aStream->WriteUInt(m_unlockedByAchievementId);
				aStream->WriteUInt(m_version);
				aStream->WriteBool(m_restricted);
				aStream->WriteObjects(m_startReputations);
				aStream->WriteObjectPointers(m_gearOptimizations);
				aStream->WriteUIntToUIntTable<uint32_t, uint32_t>(m_replaceAbilityTable);

				for(uint32_t i = 1; i < (uint32_t)ArmorStyle::NUM_IDS; i++)
					m_armorStyles[i].ToStream(aStream);

				for (uint32_t i = 1; i < (uint32_t)ItemType::NUM_IDS; i++)
					aStream->WriteOptionalObjectPointer(m_weaponSprites[i]);
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
				if (!aStream->ReadUInts(m_remains))
					return false;
				if(!aStream->ReadPOD(m_defaultArmorStyleId))
					return false;
				if(!m_armorDecoration.FromStream(aStream))
					return false;
				if (!aStream->ReadUInt(m_unlockedByAchievementId))
					return false;
				if (!aStream->ReadUInt(m_version))
					return false;
				if(!aStream->ReadBool(m_restricted))
					return false;
				if(!aStream->ReadObjects(m_startReputations))
					return false;
				if(!aStream->ReadObjectPointers(m_gearOptimizations))
					return false;
				if(!aStream->ReadUIntToUIntTable(m_replaceAbilityTable))
					return false;

				for (uint32_t i = 1; i < (uint32_t)ArmorStyle::NUM_IDS; i++)
				{
					if(!m_armorStyles[i].FromStream(aStream))
						return false;
				}

				for (uint32_t i = 1; i < (uint32_t)ItemType::NUM_IDS; i++)
				{
					if(!aStream->ReadOptionalObjectPointer(m_weaponSprites[i]))
						return false;
				}

				return true;
			}

			// Public data
			uint32_t												m_version = 0;
			std::string												m_displayName;
			std::string												m_description;
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
			std::vector<uint32_t>									m_remains;
			ArmorStyle::Id											m_defaultArmorStyleId = ArmorStyle::ID_BROWN;
			SpriteCollection										m_armorStyles[ArmorStyle::NUM_IDS];
			std::unique_ptr<SpriteCollection>						m_weaponSprites[ItemType::NUM_IDS];
			SpriteCollection										m_armorDecoration;
			uint32_t												m_unlockedByAchievementId = 0;
			bool													m_restricted = false;			
			std::vector<StartReputation>							m_startReputations;
			std::vector<std::unique_ptr<GearOptimization>>			m_gearOptimizations;
			std::unordered_map<uint32_t, uint32_t>					m_replaceAbilityTable;
		};

	}

}
