#include "Pcheader.h"

#include <tpublic/Data/Quest.h>

#include <tpublic/Manifest.h>

namespace tpublic
{

	namespace PostProcessQuests
	{

		void		
		Run(
			Manifest* aManifest)
		{
			std::unordered_set<uint32_t> questRewardItemIds;

			aManifest->GetContainer<Data::Quest>()->ForEach([aManifest, &questRewardItemIds](
				Data::Quest* aQuest)
			{
				for(uint32_t itemId : aQuest->m_rewardAllItems)
					questRewardItemIds.insert(itemId);
				for (uint32_t itemId : aQuest->m_rewardOneItem)
					questRewardItemIds.insert(itemId);
				return true;
			});

			aManifest->GetContainer<Data::Item>()->ForEach([aManifest, &questRewardItemIds](
				Data::Item* aItem)
			{
				if(questRewardItemIds.contains(aItem->m_id))
					aItem->m_flags |= Data::Item::FLAG_QUEST_REWARD;
				return true;
			});

		}

	}

}