#pragma once

#include "Data/Item.h"

#include "ItemInstance.h"
#include "Stat.h"

namespace tpublic
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

		uint32_t	GetItemLevel() const;
		uint32_t	GetWeaponCooldown() const;
	
		// Public data
		ItemInstance			m_instance;
		const Data::Item*		m_itemData = NULL;
		std::string				m_name;
		std::string				m_suffix;
		uint32_t				m_iconSpriteId = 0;
		Stat::Collection		m_stats;
		uint32_t				m_properties[Data::Item::NUM_PROPERTY_TYPES] = { 0 };
		bool					m_isWeapon = false;

	private:

		void		_Generate(						
						std::mt19937&			aRandom,
						const Manifest*			aManifest,
						const Data::Item::Node*	aNode);
	};

}