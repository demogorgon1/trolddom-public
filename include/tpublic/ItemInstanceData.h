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
	
		// Public data
		ItemInstance			m_instance;
		const Data::Item*		m_itemData = NULL;
		std::string				m_name;
		std::string				m_suffix;
		Stat::Collection		m_stats;
		uint32_t				m_vendorValue = 0;
		bool					m_isWeapon = false;
		UIntRange				m_weaponDamage;
		uint32_t				m_weaponCooldown = 0;
		uint32_t				m_cost = 0;
		int32_t					m_statBudget = 0;

	private:

		void		_Generate(						
						std::mt19937&			aRandom,
						const Manifest*			aManifest,
						const Data::Item::Node*	aNode,
						Stat::Collection&		aOutStatWeights,
						uint32_t&				aOutTotalStatWeight);
	};

}