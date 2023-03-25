#pragma once

#include "../DataBase.h"
#include "../Stat.h"

namespace kaos_public
{

	namespace Data
	{

		struct Class
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_CLASS;

			struct LevelProgressionLevel
			{
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
						else
							KP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
					});
				}

				uint32_t				m_level = 1;
				std::vector<uint32_t>	m_unlockAbilities;
				Stat::Collection		m_stats;
			};

			struct LevelProgression
			{
				LevelProgression(
					const Parser::Node* aNode)
				{
					aNode->ForEachChild([&](
						const Parser::Node* aArrayItem)
					{						
						m_levels.push_back(std::make_unique<LevelProgressionLevel>(aArrayItem->GetObject()));
					});
				}

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
						m_displayName = aMember->GetString();
					else if(aMember->m_name == "level_progression")
						m_levelProgression = std::make_unique<LevelProgression>(aMember->GetArray());
					else
						KP_VERIFY(false, aMember->m_debugInfo, "'%s' not a valid member.", aMember->m_name.c_str());
				});
			}

			// Public data
			std::string												m_displayName;
			std::unique_ptr<LevelProgression>						m_levelProgression;
		};



	}

}