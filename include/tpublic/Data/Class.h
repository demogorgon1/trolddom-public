#pragma once

#include "../ActionBar.h"
#include "../DataBase.h"
#include "../EquipmentSlot.h"
#include "../Resource.h"
#include "../Stat.h"

namespace tpublic
{

	namespace Data
	{

		struct Class
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_CLASS;

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
					const Parser::Node*		aSource)
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
					const Parser::Node*		aSource)
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
					const Parser::Node*	aNode)
				{
					m_resourceId = Resource::StringToId(aNode->m_name.c_str());
					TP_VERIFY(m_resourceId != 0, aNode->m_debugInfo, "'%s' not a valid resource.", aNode->m_name.c_str());

					aNode->ForEachChild([&](
						const Parser::Node* aMember)
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
					const Parser::Node*	aNode)
				{
					aNode->ForEachChild([&](
						const Parser::Node* aMember)
					{						
						if (aMember->m_name == "level")
							m_level = aMember->GetUInt32();
						else if (aMember->m_name == "unlock_abilities")
							aMember->GetIdArray(DataType::ID_ABILITY, m_unlockAbilities);
						else if (aMember->m_name == "stats")
							m_stats.FromSource(aMember);
						else if(aMember->m_tag == "resource")
							m_resourceUpdates.push_back(LevelProgressionLevelResourceUpdate(aMember));
						else
							TP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
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
					return true;
				}

				// Public data
				uint32_t											m_level = 1;
				std::vector<uint32_t>								m_unlockAbilities;
				Stat::Collection									m_stats;
				std::vector<LevelProgressionLevelResourceUpdate>	m_resourceUpdates;
			};

			struct LevelProgression
			{
				LevelProgression()
				{

				}

				LevelProgression(
					const Parser::Node* aNode)
				{
					aNode->ForEachChild([&](
						const Parser::Node* aArrayItem)
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

				// Public data
				std::vector<std::unique_ptr<LevelProgressionLevel>>	m_levels;
			};

			void
			Verify() const
			{
				VerifyBase();

				TP_VERIFY(!m_displayName.empty(), m_debugInfo, "'%s' has no 'string'.", m_name.c_str());
			}

			// Base implementation
			void
			FromSource(
				const Parser::Node*		aNode) override
			{
				aNode->ForEachChild([&](
					const Parser::Node* aMember)
				{
					if(aMember->m_name == "string")
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
					else if (aMember->m_name == "color_1")
					{
						const Parser::Node* components = aMember->GetArray();
						TP_VERIFY(components->m_children.size() == 3, aMember->m_debugInfo, "'%s' is not a valid color.", aMember->m_name.c_str());
						m_color1.m_r = components->m_children[0]->GetUInt8();
						m_color1.m_g = components->m_children[1]->GetUInt8();
						m_color1.m_b = components->m_children[2]->GetUInt8();
					}
					else if (aMember->m_name == "color_2")
					{
						const Parser::Node* components = aMember->GetArray();
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
					else if(aMember->m_name == "level_progression")
					{
						m_levelProgression = std::make_unique<LevelProgression>(aMember->GetArray());
					}
					else if(aMember->m_tag == "start_equipment")
					{
						m_startEquipment.push_back(StartEquipment(aMember));
					}
					else if (aMember->m_tag == "start_map")
					{
						m_startMaps.push_back(std::make_unique<StartMap>(aMember));
					}
					else
					{
						TP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteString(m_displayName);
				aStream->WriteString(m_description);
				aStream->WriteOptionalObjectPointer(m_levelProgression);
				aStream->WriteUInt(m_spriteId);
				aStream->WriteUInt(m_defaultAttackAbilityId);
				aStream->WriteObjects(m_startEquipment);
				m_defaultActionBar.ToStream(aStream);
				aStream->WritePOD(m_color1);
				aStream->WritePOD(m_color2);
				aStream->WriteObjectPointers(m_startMaps);
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadString(m_displayName))
					return false;
				if (!aStream->ReadString(m_description))
					return false;
				if(!aStream->ReadOptionalObjectPointer(m_levelProgression))
					return false;
				if(!aStream->ReadUInt(m_spriteId))
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
				return true;
			}

			// Public data
			std::string												m_displayName;
			std::string												m_description;
			uint32_t												m_spriteId = 0;
			Color													m_color1;
			Color													m_color2;
			uint32_t												m_defaultAttackAbilityId = 0;
			std::unique_ptr<LevelProgression>						m_levelProgression;
			std::vector<StartEquipment>								m_startEquipment;
			ActionBar												m_defaultActionBar;
			std::vector<std::unique_ptr<StartMap>>					m_startMaps;
		};

	}

}
