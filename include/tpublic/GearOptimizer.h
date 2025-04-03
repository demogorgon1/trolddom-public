#pragma once

#include "Components/EquippedItems.h"

#include "Data/Class.h"
#include "Data/Item.h"

#include "Rarity.h"

namespace tpublic
{

	class Manifest;

	class GearOptimizer
	{
	public:
				GearOptimizer(
					const Manifest*							aManifest);
				~GearOptimizer();

		void	MakeEquippedItems(
					Rarity::Id								aMaxRarity,
					uint32_t								aMaxLevel,					
					uint32_t								aFlags,
					const Data::Class*						aClass,
					const char*								aGearOptimizationName,
					Components::EquippedItems::Slots&		aOut);

	private:

		const Manifest*		m_manifest;

		float	_ScoreItem(
					const Data::Item*						aItem,
					const Data::Class::GearOptimization*	aGearOptimization) const;
	};

}