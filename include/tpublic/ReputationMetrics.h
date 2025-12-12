#pragma once

#include "Image.h"
#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"

namespace tpublic
{

	class ReputationMetrics
	{
	public:		
		struct Level
		{
			Level()
			{

			}

			Level(
				const SourceNode*			aSource)
			{
				TP_VERIFY(aSource->GetArray()->m_children.size() == 3, aSource->m_debugInfo, "Invalid level entry.");
				m_reputation = aSource->GetArrayIndex(0)->GetInt32();
				m_string = aSource->GetArrayIndex(1)->GetString();
				TP_VERIFY(aSource->GetArrayIndex(2)->GetArray()->m_children.size() == 3, aSource->m_debugInfo, "Invalid level color list.");
				m_color1 = aSource->GetArrayIndex(2)->GetArrayIndex(0);
				m_color2 = aSource->GetArrayIndex(2)->GetArrayIndex(1);
				m_color3 = aSource->GetArrayIndex(2)->GetArrayIndex(2);
			}

			void
			ToStream(
				IWriter*					aWriter) const
			{
				aWriter->WriteInt(m_reputation);
				aWriter->WriteString(m_string);
				aWriter->WritePOD(m_color1);
				aWriter->WritePOD(m_color2);
				aWriter->WritePOD(m_color3);
			}

			bool
			FromStream(
				IReader*					aReader)
			{
				if(!aReader->ReadInt(m_reputation))
					return false;
				if (!aReader->ReadString(m_string))
					return false;
				if (!aReader->ReadPOD(m_color1))
					return false;
				if (!aReader->ReadPOD(m_color2))
					return false;
				if (!aReader->ReadPOD(m_color3))
					return false;
				return true;
			}

			// Public data
			int32_t			m_reputation = 0;
			std::string		m_string;
			Image::RGBA		m_color1;
			Image::RGBA		m_color2;
			Image::RGBA		m_color3;
		};

		void
		FromSource(
			const SourceNode*				aSource)
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "base_quest_reputation")
				{
					m_baseQuestReputation = aChild->GetInt32();
				}
				else if(aChild->m_name == "levels")
				{
					m_levels.clear();

					aChild->ForEachChild([&](
						const SourceNode* aLevelEntry)
					{
						m_levels.push_back(Level(aLevelEntry));
					});

					TP_VERIFY(m_levels.size() > 0, aChild->m_debugInfo, "No levels defined.");

					std::sort(m_levels.begin(), m_levels.end(), [](
						const Level& aLHS,
						const Level& aRHS)
					{
						return aLHS.m_reputation < aRHS.m_reputation;
					});

					m_minReputation = m_levels[0].m_reputation;
					m_maxReputation = m_levels[m_levels.size() - 1].m_reputation;
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
			aStream->WriteInt(m_baseQuestReputation);
			aStream->WriteObjects(m_levels);
			aStream->WriteInt(m_minReputation);
			aStream->WriteInt(m_maxReputation);
		}

		bool
		FromStream(
			IReader*						aStream) 
		{
			if (!aStream->ReadInt(m_baseQuestReputation))
				return false;
			if(!aStream->ReadObjects(m_levels))
				return false;
			if (!aStream->ReadInt(m_minReputation))
				return false;
			if (!aStream->ReadInt(m_maxReputation))
				return false;
			return true;
		}

		uint32_t
		GetLevelIndexFromReputation(
			int32_t							aReputation) const
		{
			for(size_t i = 0; i < m_levels.size(); i++)
			{
				size_t j = m_levels.size() - i - 1;
				const Level& level = m_levels[j];
				if (aReputation >= level.m_reputation)
					return (uint32_t)j;
			}
			return 0;
		}

		const Level&
		GetLevelFromReputation(
			int32_t							aReputation) const
		{	
			assert(m_levels.size() > 0);
			return m_levels[GetLevelIndexFromReputation(aReputation)];
		}
				
		// Public data
		int32_t				m_baseQuestReputation = 0;
		std::vector<Level>	m_levels;
		int32_t				m_maxReputation = 0;
		int32_t				m_minReputation = 0;
	};

}
