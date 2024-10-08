#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace tpublic
{

	class XPMetrics
	{
	public:
		struct LevelDiffColor
		{
			void
			ToStream(
				IWriter*		aWriter) const
			{
				aWriter->WritePOD(m_color[0]);
				aWriter->WritePOD(m_color[1]);
				aWriter->WritePOD(m_color[2]);
			}

			bool
			FromStream(
				IReader*		aReader) 
			{
				if (!aReader->ReadPOD(m_color[0]))
					return false;
				if (!aReader->ReadPOD(m_color[1]))
					return false;
				if (!aReader->ReadPOD(m_color[2]))
					return false;
				return true;
			}

			// Public data
			uint8_t			m_color[3];
		};

		XPMetrics()
		{

		}	

		void
		FromSource(
			const SourceNode*	aSource)
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "levels")
				{
					aChild->GetArray()->ForEachChild([&](
						const SourceNode* aEntry)
					{
						TP_VERIFY(aEntry->m_type == SourceNode::TYPE_ARRAY && aEntry->m_children.size() == 2, aEntry->m_debugInfo, "Not a level-xp pair.");
						uint32_t level = aEntry->m_children[0]->GetUInt32();
						uint32_t xp = aEntry->m_children[1]->GetUInt32();

						if(level >= m_xpToLevel.size())
							m_xpToLevel.resize(level + 1);

						m_xpToLevel[level] = xp;

						if(level > m_maxLevel)
							m_maxLevel = level;
					});
				}
				else if(aChild->m_name == "kills")
				{
					aChild->GetArray()->ForEachChild([&](
						const SourceNode* aEntry)
					{
						TP_VERIFY(aEntry->m_type == SourceNode::TYPE_ARRAY && aEntry->m_children.size() == 2, aEntry->m_debugInfo, "Not a level-xp pair.");
						uint32_t level = aEntry->m_children[0]->GetUInt32();
						uint32_t xp = aEntry->m_children[1]->GetUInt32();

						if(level >= m_xpFromKill.size())
							m_xpFromKill.resize(level + 1);

						m_xpFromKill[level] = xp;
					});
				}
				else if(aChild->m_name == "quests")
				{
					aChild->GetArray()->ForEachChild([&](
						const SourceNode* aEntry)
					{
						TP_VERIFY(aEntry->m_type == SourceNode::TYPE_ARRAY && aEntry->m_children.size() == 2, aEntry->m_debugInfo, "Not a level-xp pair.");
						uint32_t level = aEntry->m_children[0]->GetUInt32();
						uint32_t xp = aEntry->m_children[1]->GetUInt32();

						if(level >= m_xpFromQuest.size())
							m_xpFromQuest.resize(level + 1);

						m_xpFromQuest[level] = xp;
					});
				}
				else if(aChild->m_name == "adjustments")
				{
					TP_VERIFY(aChild->GetArray()->m_children.size() > 0, aChild->m_debugInfo, "Array is empty.");
					m_minAdjustment = 0;
					m_minAdjustmentLevelDiff = INT32_MAX;
					m_maxAdjustment = 0;
					m_maxAdjustmentLevelDiff = INT32_MIN;

					aChild->GetArray()->ForEachChild([&](
						const SourceNode* aEntry)
					{
						TP_VERIFY(aEntry->m_type == SourceNode::TYPE_ARRAY && aEntry->m_children.size() == 3, aEntry->m_debugInfo, "Not a level difference-percentage-color.");
						int32_t levelDiff = aEntry->m_children[0]->GetInt32();
						uint32_t adjustment = aEntry->m_children[1]->GetUInt32();

						LevelDiffColor levelDiffColor;
						levelDiffColor.m_color[0] = aEntry->m_children[2]->GetArrayIndex(0)->GetUInt8();
						levelDiffColor.m_color[1] = aEntry->m_children[2]->GetArrayIndex(1)->GetUInt8();
						levelDiffColor.m_color[2] = aEntry->m_children[2]->GetArrayIndex(2)->GetUInt8();

						if(levelDiff < m_minAdjustmentLevelDiff)
						{
							m_minAdjustmentLevelDiff = levelDiff;
							m_minAdjustment = adjustment;
							m_minLevelDiffColor = levelDiffColor;
						}

						if (levelDiff > m_maxAdjustmentLevelDiff)
						{
							m_maxAdjustmentLevelDiff = levelDiff;
							m_maxAdjustment = adjustment;
							m_maxLevelDiffColor = levelDiffColor;
						}

						m_adjustments[levelDiff] = adjustment;
						m_levelDiffColors[levelDiff] = levelDiffColor;
					});
				}
				else if (aChild->m_name == "elite_kill_adjustment")
				{
					m_eliteKillAdjustment = aChild->GetUInt32();
				}
				else if (aChild->m_name == "elite_quest_adjustment")
				{
					m_eliteQuestAdjustment = aChild->GetUInt32();
				}
				else if (aChild->m_name == "max_level_demo")
				{
					m_maxLevelDemo = aChild->GetUInt32();
				}
				else
				{
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				}
			});
		}

		void
		ToStream(
			IWriter*			aStream) const 
		{
			aStream->WriteUInts(m_xpToLevel);
			aStream->WriteUInts(m_xpFromKill);
			aStream->WriteUInts(m_xpFromQuest);
			aStream->WriteInt(m_minAdjustmentLevelDiff);
			aStream->WriteInt(m_maxAdjustmentLevelDiff);
			aStream->WriteUInt(m_minAdjustment);
			aStream->WriteUInt(m_maxAdjustment);
			aStream->WriteUInt(m_maxLevel);
			aStream->WriteUInt(m_maxLevelDemo);
			aStream->WriteUInt(m_eliteKillAdjustment);
			aStream->WriteUInt(m_eliteQuestAdjustment);
			m_minLevelDiffColor.ToStream(aStream);
			m_maxLevelDiffColor.ToStream(aStream);

			aStream->WriteUInt(m_adjustments.size());
			for(std::unordered_map<int32_t, uint32_t>::const_iterator i = m_adjustments.cbegin(); i != m_adjustments.cend(); i++)
			{
				aStream->WriteInt(i->first);
				aStream->WriteUInt(i->second);
			}

			aStream->WriteUInt(m_levelDiffColors.size());
			for (std::unordered_map<int32_t, LevelDiffColor>::const_iterator i = m_levelDiffColors.cbegin(); i != m_levelDiffColors.cend(); i++)
			{
				aStream->WriteInt(i->first);
				i->second.ToStream(aStream);
			}
		}

		bool
		FromStream(
			IReader*			aStream) 
		{
			if(!aStream->ReadUInts(m_xpToLevel))
				return false;
			if (!aStream->ReadUInts(m_xpFromKill))
				return false;
			if (!aStream->ReadUInts(m_xpFromQuest))
				return false;
			if(!aStream->ReadInt(m_minAdjustmentLevelDiff))
				return false;
			if (!aStream->ReadInt(m_maxAdjustmentLevelDiff))
				return false;
			if (!aStream->ReadUInt(m_minAdjustment))
				return false;
			if (!aStream->ReadUInt(m_maxAdjustment))
				return false;
			if (!aStream->ReadUInt(m_maxLevel))
				return false;
			if (!aStream->ReadUInt(m_maxLevelDemo))
				return false;
			if (!aStream->ReadUInt(m_eliteKillAdjustment))
				return false;
			if (!aStream->ReadUInt(m_eliteQuestAdjustment))
				return false;
			if (!m_minLevelDiffColor.FromStream(aStream))
				return false;
			if(!m_maxLevelDiffColor.FromStream(aStream))
				return false;

			{
				size_t count;
				if(!aStream->ReadUInt(count))
					return false;

				for(size_t i = 0; i < count; i++)
				{
					int32_t key;
					if(!aStream->ReadInt(key))
						return false;

					uint32_t value;
					if(!aStream->ReadUInt(value))
						return false;

					m_adjustments[key] = value;
				}
			}

			{
				size_t count;
				if(!aStream->ReadUInt(count))
					return false;

				for(size_t i = 0; i < count; i++)
				{
					int32_t key;
					if(!aStream->ReadInt(key))
						return false;

					LevelDiffColor value;
					if(!value.FromStream(aStream))
						return false;

					m_levelDiffColors[key] = value;
				}
			}

			return true;
		}
		
		uint32_t
		GetXPToLevel(
			uint32_t			aLevel) const
		{
			if(aLevel >= m_xpToLevel.size())
				return 0;
			return m_xpToLevel[aLevel];
		}

		uint32_t
		GetXPFromKill(
			uint32_t			aLevel) const
		{
			if (aLevel >= m_xpFromKill.size())
				return 0;
			return m_xpFromKill[aLevel];
		}

		uint32_t
		GetXPFromQuest(
			uint32_t			aLevel) const
		{
			if (aLevel >= m_xpFromQuest.size())
				return 0;
			return m_xpFromQuest[aLevel];
		}

		uint32_t 
		GetAdjustment(
			int32_t				aLevelDiff) const
		{
			if(aLevelDiff == 0)
				return 100;
			if (aLevelDiff <= m_minAdjustmentLevelDiff)
				return m_minAdjustment;
			if(aLevelDiff >= m_maxAdjustmentLevelDiff)
				return m_maxAdjustment;

			std::unordered_map<int32_t, uint32_t>::const_iterator i = m_adjustments.find(aLevelDiff);
			if(i == m_adjustments.end())
				return 0;
			return i->second;
		}

		void
		GetLevelDiffColor(
			int32_t				aLevelDiff,
			uint8_t				aOut[3]) const
		{
			if (aLevelDiff <= m_minAdjustmentLevelDiff)
			{
				aOut[0] = m_minLevelDiffColor.m_color[0];
				aOut[1] = m_minLevelDiffColor.m_color[1];
				aOut[2] = m_minLevelDiffColor.m_color[2];
			}
			else if (aLevelDiff >= m_maxAdjustmentLevelDiff)
			{
				aOut[0] = m_maxLevelDiffColor.m_color[0];
				aOut[1] = m_maxLevelDiffColor.m_color[1];
				aOut[2] = m_maxLevelDiffColor.m_color[2];
			}
			else
			{
				std::unordered_map<int32_t, LevelDiffColor>::const_iterator i = m_levelDiffColors.find(aLevelDiff);
				if (i == m_levelDiffColors.end())
				{
					aOut[0] = m_minLevelDiffColor.m_color[0];
					aOut[1] = m_minLevelDiffColor.m_color[1];
					aOut[2] = m_minLevelDiffColor.m_color[2];
				}
				else
				{
					aOut[0] = i->second.m_color[0];
					aOut[1] = i->second.m_color[1];
					aOut[2] = i->second.m_color[2];
				}
			}
		}

		uint32_t
		GetAdjustedXPFromKill(
			uint32_t			aPlayerLevel,
			uint32_t			aKillLevel) const
		{
			// FIXME: include elite flag
			int32_t diff = (int32_t)aKillLevel - (int32_t)aPlayerLevel;
			uint32_t adjustment = GetAdjustment(diff);
			return (GetXPFromKill(aKillLevel) * adjustment) / 100;
		}

		uint32_t
		GetAdjustedXPFromQuest(
			uint32_t			aPlayerLevel,
			uint32_t			aQuestLevel,
			bool				aElite) const
		{
			int32_t diff = (int32_t)aQuestLevel - (int32_t)aPlayerLevel;
			uint32_t adjustment = GetAdjustment(diff);
			uint32_t xp = (GetXPFromQuest(aQuestLevel) * adjustment) / 100;

			if(aElite)
				xp = (xp * m_eliteQuestAdjustment) / 100;

			return xp;
		}
		
		// Public data
		uint32_t									m_maxLevel = 0;
		uint32_t									m_maxLevelDemo = 0;
		std::vector<uint32_t>						m_xpToLevel;
		std::vector<uint32_t>						m_xpFromKill;
		std::vector<uint32_t>						m_xpFromQuest;
		int32_t										m_minAdjustmentLevelDiff = 0;
		int32_t										m_maxAdjustmentLevelDiff = 0;
		uint32_t									m_minAdjustment = 0;
		uint32_t									m_maxAdjustment = 0;
		std::unordered_map<int32_t, uint32_t>		m_adjustments;
		LevelDiffColor								m_minLevelDiffColor;
		LevelDiffColor								m_maxLevelDiffColor;
		std::unordered_map<int32_t, LevelDiffColor>	m_levelDiffColors;
		uint32_t									m_eliteKillAdjustment = 100;
		uint32_t									m_eliteQuestAdjustment = 100;
	};

}