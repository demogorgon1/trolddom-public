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
						const Manifest*			aManifest,
						const ItemInstance&		aItemInstance);
					~ItemInstanceData();
	
		// Public data
		ItemInstance			m_instance;
		const Data::Item*		m_itemData = NULL;
		std::string				m_name;
		std::string				m_suffix;
		uint32_t				m_iconSpriteId = 0;
		Stat::Collection		m_stats;
		uint32_t				m_properties[Data::Item::NUM_PROPERTY_TYPES] = { 0 };

	private:

		void		_Generate(						
						std::mt19937&			aRandom,
						const Manifest*			aManifest,
						const Data::Item::Node*	aNode);
	};

}