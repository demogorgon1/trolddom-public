#pragma once

#include "../DataBase.h"
#include "../Resource.h"
#include "../Stat.h"

namespace kpublic
{

	namespace Data
	{

		struct Class
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_CLASS;

			struct LevelProgressionLevelResourceUpdate
			{	
				LevelProgressionLevelResourceUpdate()
				{

				}

				LevelProgressionLevelResourceUpdate(
					const Parser::Node*	aNode)
				{
					m_resourceId = Resource::StringToId(aNode->m_name.c_str());
					KP_VERIFY(m_resourceId != 0, aNode->m_debugInfo, "'%s' not a valid resource.", aNode->m_name.c_str());

					aNode->ForEachChild([&](
						const Parser::Node* aMember)
					{						
						if (aMember->m_name == "add_max")
							m_addMax = aMember->GetUInt32();
						else
							KP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
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
							KP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
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

				KP_VERIFY(!m_displayName.empty(), m_debugInfo, "'%s' has no 'display_name'.", m_name.c_str());
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
					else if (aMember->m_name == "sprite")
					{
						m_spriteId = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aMember->GetIdentifier());
					}					
					else if (aMember->m_name == "main_attack")
					{
						m_mainAttackAbilityId = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY, aMember->GetIdentifier());
					}
					else if(aMember->m_name == "level_progression")
					{
						m_levelProgression = std::make_unique<LevelProgression>(aMember->GetArray());
					}
					else
					{
						KP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteString(m_displayName);
				aStream->WriteOptionalObjectPointer(m_levelProgression);
				aStream->WriteUInt(m_spriteId);
				aStream->WriteUInt(m_mainAttackAbilityId);
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadString(m_displayName))
					return false;
				if(!aStream->ReadOptionalObjectPointer(m_levelProgression))
					return false;
				if(!aStream->ReadUInt(m_spriteId))
					return false;
				if (!aStream->ReadUInt(m_mainAttackAbilityId))
					return false;
				return true;
			}

			// Public data
			std::string												m_displayName;
			uint32_t												m_spriteId = 0;
			uint32_t												m_mainAttackAbilityId = 0;
			std::unique_ptr<LevelProgression>						m_levelProgression;
		};

	}

}
