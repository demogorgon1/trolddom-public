#pragma once

#include "Data/Item.h"

#include "ItemInstance.h"
#include "Stat.h"

namespace kpublic
{

	class Manifest;

	class ItemInstanceData
	{
	public:
					ItemInstanceData();
					ItemInstanceData(
						std::mt19937&			aRandom,
						const Manifest*			aManifest,
						const ItemInstance&		aItemInstance);
					~ItemInstanceData();
	
		// Public data
		ItemInstance			m_instance;
		const Data::Item*		m_itemData = NULL;
		std::string				m_string;
		Stat::Collection		m_stats;
		uint32_t				m_properties[Data::Item::NUM_PROPERTY_TYPES];

	private:

		void		_Generate(						
						std::mt19937&			aRandom,
						const Manifest*			aManifest,
						const Data::Item::Node*	aNode);
	};

}