#pragma once

#include <tpublic/Data/Ability.h>

#include "Requirement.h"

namespace tpublic
{

	class EntityInstance;
	class ItemProspect;
	class Manifest;

	namespace Requirements
	{

		bool	Check(
					const Manifest*										aManifest,
					const EntityInstance*								aSelf,
					const EntityInstance*								aTarget,
					const Requirement*									aRequirement);
		bool	CheckList(
					const Manifest*										aManifest,
					const std::vector<Requirement>&						aRequirements,
					const EntityInstance*								aSelf,
					const EntityInstance*								aTarget,
					const Requirement**									aOutFailedRequirement = NULL);
		bool	CheckAnyList(
					const Manifest*										aManifest,
					const std::vector<Requirement>&						aRequirements,
					const std::vector<const EntityInstance*>&			aSelves,
					const EntityInstance*								aTarget);
		bool	CheckOpenable(
					const Manifest*										aManifest,
					const EntityInstance*								aSelf,
					const EntityInstance*								aTarget,
					bool*												aOutInstant,
					bool*												aOutOutOfRange);
		bool	CheckTargetItemRequirements(
					const Manifest*										aManifest,
					const Data::Ability::TargetItemRequirements*		aTargetItemRequirements,
					const ItemProspect*									aTargetItemProspect,
					uint32_t											aItemId);
	}

}