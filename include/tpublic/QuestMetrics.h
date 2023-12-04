#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace tpublic
{

	class QuestMetrics
	{
	public:		
		QuestMetrics()
		{

		}

		void
		FromSource(
			const SourceNode*			aSource)
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "elite_cash_adjustment")
				{
					m_eliteCashAdjustment = aChild->GetUInt32();
				}
				else if(aChild->m_name == "cash_rewards_per_level")
				{
					aChild->GetArray()->ForEachChild([&](
						const SourceNode* aItem)
					{
						TP_VERIFY(aItem->m_children.size() == 2, aChild->m_debugInfo, "Not a valid level-cash pair.");
						uint32_t level = aItem->GetArrayIndex(0)->GetUInt32();
						uint32_t cash = aItem->GetArrayIndex(1)->GetUInt32();

						while(m_cashRewardForLevel.size() <= level)
							m_cashRewardForLevel.push_back(0);

						m_cashRewardForLevel[level] = cash;
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
			aStream->WriteUInts(m_cashRewardForLevel);
			aStream->WriteUInt(m_eliteCashAdjustment);
		}

		bool
		FromStream(
			IReader*						aStream) 
		{
			if(!aStream->ReadUInts(m_cashRewardForLevel))
				return false;
			if(!aStream->ReadUInt(m_eliteCashAdjustment))
				return false;
			return true;
		}

		uint32_t
		GetCashRewardForLevel(
			uint32_t						aLevel,
			bool							aElite) const
		{	
			if(m_cashRewardForLevel.size() == 0)
				return 0;

			uint32_t cash = 0;

			if((size_t)aLevel >= m_cashRewardForLevel.size())
				cash = m_cashRewardForLevel[m_cashRewardForLevel.size() - 1];
			else
				cash = m_cashRewardForLevel[aLevel];

			if(aElite)
				cash = (cash * m_eliteCashAdjustment) / 100;

			return cash;
		}
				
		// Public data
		std::vector<uint32_t>			m_cashRewardForLevel;
		uint32_t						m_eliteCashAdjustment = 100;
	};

}
