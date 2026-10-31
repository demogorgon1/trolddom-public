#pragma once

#include "Components/Talents.h"

#include "Data/Class.h"

#include "ItemType.h"

namespace tpublic
{

	class Manifest;

	// A hybrid is a character with one of its class' talent trees replaced by a talent tree from another class (the
	// donor class). These helpers combine the two classes. A hybrid talent tree id of 0 means not a hybrid.
	namespace HybridClass
	{

		struct LevelProgression
		{
			// Class level progression up to the character's level
			std::vector<const Data::Class::LevelProgressionLevel*>		m_levels;

			// Donor class 'hybrid' level progression up to the character's level. Only resources and abilities are used.
			std::vector<const Data::Class::LevelProgressionLevel*>		m_hybridLevels;

			// Abilities from both
			std::vector<uint32_t>										m_abilityIds;
		};

		const Data::Class*	GetDonorClass(
								const Manifest*								aManifest,
								uint32_t									aHybridTalentTreeId);
		bool				IsValid(
								const Manifest*								aManifest,
								const Data::Class*							aClass,
								const Components::Talents*					aTalents);
		bool				CanUseItemType(
								const Manifest*								aManifest,
								const Data::Class*							aClass,
								uint32_t									aHybridTalentTreeId,
								ItemType::Id								aItemType);
		void				GetSystems(
								const Manifest*								aManifest,
								const Data::Class*							aClass,
								uint32_t									aHybridTalentTreeId,
								std::vector<uint32_t>&						aOut);
		void				GetStatsConversion(
								const Manifest*								aManifest,
								const Data::Class*							aClass,
								uint32_t									aHybridTalentTreeId,
								Data::Class::StatsConversion&				aOut);
		void				GetLevelProgression(
								const Manifest*								aManifest,
								const Data::Class*							aClass,
								const Components::Talents*					aTalents,
								uint32_t									aLevel,
								LevelProgression&							aOut);

	}

}
