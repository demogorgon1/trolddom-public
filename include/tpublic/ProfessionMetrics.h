#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace tpublic
{

	class ProfessionMetrics
	{
	public:		
		struct Level
		{
			Level()
			{

			}

			Level(
				const SourceNode*		aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "string")					
						m_string = aChild->GetString();
					else if (aChild->m_name == "required_level")
						m_requiredLevel = aChild->GetUInt32();
					else if (aChild->m_name == "required_skill")
						m_requiredSkill = aChild->GetUInt32();
					else if (aChild->m_name == "max_skill")
						m_maxSkill = aChild->GetUInt32();
					else if (aChild->m_name == "training_cost")
						m_trainingCost = aChild->GetUInt32();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*				aWriter) const
			{
				aWriter->WriteString(m_string);
				aWriter->WriteUInt(m_requiredLevel);
				aWriter->WriteUInt(m_requiredSkill);
				aWriter->WriteUInt(m_maxSkill);
				aWriter->WriteUInt(m_trainingCost);
			}

			bool
			FromStream(
				IReader*				aReader)
			{
				if(!aReader->ReadString(m_string))
					return false;
				if (!aReader->ReadUInt(m_requiredLevel))
					return false;
				if (!aReader->ReadUInt(m_requiredSkill))
					return false;
				if (!aReader->ReadUInt(m_maxSkill))
					return false;
				if (!aReader->ReadUInt(m_trainingCost))
					return false;
				return true;
			}

			// Public data				
			std::string	m_string;
			uint32_t	m_requiredLevel = 0;
			uint32_t	m_requiredSkill = 0;
			uint32_t	m_maxSkill = 0;
			uint32_t	m_trainingCost = 1;
		};

		struct Skills
		{
			void
			FromSource(
				const SourceNode*			aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "skill_up_chance")
					{
						TP_VERIFY(aChild->m_annotation, aChild->m_debugInfo, "Missing skill diferrence annotation.");
						int32_t difference = aChild->m_annotation->GetInt32();
						m_table[difference] = aChild->GetUInt32();
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
				aStream->WriteUInt(m_table.size());
				for(Table::const_iterator i = m_table.cbegin(); i != m_table.cend(); i++)
				{
					aStream->WriteInt(i->first);
					aStream->WriteUInt(i->second);
				}
			}
	
			bool
			FromStream(
				IReader*						aStream) 
			{
				size_t count;
				if(!aStream->ReadUInt(count))
					return false;
				for(size_t i = 0; i < count; i++)
				{
					int32_t difference;
					if(!aStream->ReadInt(difference))
						return false;

					uint32_t chance;
					if (!aStream->ReadUInt(chance))
						return false;

					m_table[difference] = chance;

				}
				return true;
			}
	
			// Public data
			typedef std::unordered_map<int32_t, uint32_t> Table;
			Table		m_table;
		};

		void
		FromSource(
			const SourceNode*			aSource)
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "levels")
				{
					aChild->GetArray()->ForEachChild([&](
						const SourceNode* aItem)
					{
						m_levels.push_back(Level(aItem));
					});
				}
				else if (aChild->m_name == "skills")
				{
					m_skills.FromSource(aChild);
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
			aStream->WriteObjects(m_levels);
			m_skills.ToStream(aStream);
		}

		bool
		FromStream(
			IReader*						aStream) 
		{
			if(!aStream->ReadObjects(m_levels))
				return false;
			if(!m_skills.FromStream(aStream))
				return false;
			return true;
		}

		bool
		IsValidLevel(
			uint32_t						aLevel) const
		{
			return (size_t)aLevel < m_levels.size();
		}

		const Level*
		GetLevel(
			uint32_t						aLevel) const
		{
			if((size_t)aLevel < m_levels.size())
				return &m_levels[aLevel];
			return NULL;
		}

		uint32_t 
		GetSkillUpChance(
			uint32_t						aSkill,
			uint32_t						aSkillRequired) const
		{
			int32_t difference = (int32_t)aSkillRequired - (int32_t)aSkill;
			Skills::Table::const_iterator i = m_skills.m_table.find(difference);
			if(i == m_skills.m_table.cend())
				return 0;
			return i->second;
		}

		// Public data
		std::vector<Level>				m_levels;		
		Skills							m_skills;
	};

}
