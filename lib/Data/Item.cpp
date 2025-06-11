#include "../Pcheader.h"

#include <tpublic/Data/Item.h>

#include <tpublic/Manifest.h>

namespace tpublic::Data
{

	void
	Item::PrepareRuntime(
		uint8_t					/*aRuntime*/,
		const Manifest*			aManifest)
	{
		std::vector<AddedStat> replacementStats;

		for (size_t i = 0; i < m_addedStats.size(); i++)
		{
			const AddedStat& addedStat = m_addedStats[i];

			AddedStat replacementStat;
			if (aManifest->m_itemMetrics.m_invalidStats.ShouldReplaceStat(m_itemType, m_equipmentSlots, addedStat.m_id, addedStat.m_value, replacementStat.m_id, replacementStat.m_value))
			{
				// Budget weights are just transfered directly while raw stats need to be converted based on cost
				if (addedStat.m_isBudgetWeight)
				{
					replacementStat.m_value = addedStat.m_value;
					replacementStat.m_isBudgetWeight = true;
				}

				replacementStats.push_back(replacementStat);

				m_addedStats.erase(m_addedStats.begin() + i);
				i--;
			}
		}

		for (const AddedStat& replacementStat : replacementStats)
		{
			// Already has this one? 
			bool alreadyThere = false;
			for (AddedStat& addedStat : m_addedStats)
			{
				if (addedStat.m_id == replacementStat.m_id && addedStat.m_isBudgetWeight == replacementStat.m_isBudgetWeight)
				{
					addedStat.m_value += replacementStat.m_value;
					alreadyThere = true;
					break;
				}
			}

			if (!alreadyThere)
				m_addedStats.push_back(replacementStat);
		}
	}
}