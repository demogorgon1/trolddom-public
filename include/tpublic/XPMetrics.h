#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace tpublic
{

	class XPMetrics
	{
	public:
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
						TP_VERIFY(aEntry->m_type == SourceNode::TYPE_ARRAY && aEntry->m_children.size() == 2, aEntry->m_debugInfo, "Not a level difference-percentage pair.");
						int32_t levelDiff = aEntry->m_children[0]->GetInt32();
						uint32_t adjustment = aEntry->m_children[1]->GetUInt32();

						if(levelDiff < m_minAdjustmentLevelDiff)
						{
							m_minAdjustmentLevelDiff = levelDiff;
							m_minAdjustment = adjustment;
						}

						if (levelDiff > m_maxAdjustmentLevelDiff)
						{
							m_maxAdjustmentLevelDiff = levelDiff;
							m_maxAdjustment = adjustment;
						}

						m_adjustments[levelDiff] = adjustment;
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
			aStream->WriteUInt(m_eliteKillAdjustment);
			aStream->WriteUInt(m_eliteQuestAdjustment);

			aStream->WriteUInt(m_adjustments.size());
			for(std::unordered_map<int32_t, uint32_t>::const_iterator i = m_adjustments.cbegin(); i != m_adjustments.cend(); i++)
			{
				aStream->WriteInt(i->first);
				aStream->WriteUInt(i->second);
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
			if (!aStream->ReadUInt(m_eliteKillAdjustment))
				return false;
			if (!aStream->ReadUInt(m_eliteQuestAdjustment))
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
		
		// Public data
		uint32_t								m_maxLevel = 0;
		std::vector<uint32_t>					m_xpToLevel;
		std::vector<uint32_t>					m_xpFromKill;
		std::vector<uint32_t>					m_xpFromQuest;
		int32_t									m_minAdjustmentLevelDiff = 0;
		int32_t									m_maxAdjustmentLevelDiff = 0;
		uint32_t								m_minAdjustment = 0;
		uint32_t								m_maxAdjustment = 0;
		std::unordered_map<int32_t, uint32_t>	m_adjustments;
		uint32_t								m_eliteKillAdjustment = 100;
		uint32_t								m_eliteQuestAdjustment = 100;
	};

}