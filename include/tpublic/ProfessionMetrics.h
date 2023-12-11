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
		}

		bool
		FromStream(
			IReader*						aStream) 
		{
			if(!aStream->ReadObjects(m_levels))
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

		// Public data
		std::vector<Level>				m_levels;
	};

}
