#include "Pcheader.h"

#include <tpublic/ItemInstanceData.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	ItemInstanceData::ItemInstanceData()
	{
	
	}

	ItemInstanceData::ItemInstanceData(
		const Manifest*		aManifest,
		const ItemInstance&	aItemInstance)
		: m_instance(aItemInstance)
	{
		m_itemData = aManifest->m_items.GetById(aItemInstance.m_itemId);
		std::mt19937 random(aItemInstance.m_seed);

		if(m_itemData->m_root)
			_Generate(random, aManifest, m_itemData->m_root.get());
	}
	
	ItemInstanceData::~ItemInstanceData()
	{

	}

	//---------------------------------------------------------

	void		
	ItemInstanceData::_Generate(
		std::mt19937&			aRandom,
		const Manifest*			aManifest,
		const Data::Item::Node*	aNode)
	{
		if(!aNode->m_name.empty())
			m_name = aNode->m_name;

		if (!aNode->m_suffix.empty())
			m_suffix = aNode->m_suffix;

		if(aNode->m_iconSpriteId != 0)
			m_iconSpriteId = aNode->m_iconSpriteId;

		for(const Data::Item::Property& p : aNode->m_properties)
			m_properties[p.m_type] = p.m_value;

		for(const Data::Item::AddedStat& addedStat : aNode->m_addedStats)
			m_stats.m_stats[addedStat.m_id] += Helpers::RandomInRange<uint32_t>(aRandom, addedStat.m_min, addedStat.m_max);

		for(const std::unique_ptr<Data::Item::Node>& child : aNode->m_randomChildren)
		{
			if(aRandom() <= child->m_chance)
				_Generate(aRandom, aManifest, child.get());
		}

		if(aNode->m_weightedChildren.size() > 0)
		{
			uint32_t sum = 0;
			uint32_t roll = Helpers::RandomInRange<uint32_t>(aRandom, 0, aNode->m_totalChildWeight);
			const Data::Item::Node* chosen = NULL;

			for(size_t i = 0; i < aNode->m_weightedChildren.size() && chosen == NULL; i++)
			{
				const std::unique_ptr<Data::Item::Node>& child = aNode->m_weightedChildren[i];
				sum += child->m_weight;
				if(roll <= sum)
					chosen = child.get();
			}

			assert(chosen != NULL);

			_Generate(aRandom, aManifest, chosen);
		}
	}

}