#pragma once

#include "Requirement.h"

namespace tpublic
{

	class EntityInstance;

	namespace Requirements
	{

		bool	Check(
					const EntityInstance*								aSelf,
					const EntityInstance*								aTarget,
					const Requirement*									aRequirement);
		bool	CheckList(
					const std::vector<Requirement>&						aRequirements,
					const EntityInstance*								aSelf,
					const EntityInstance*								aTarget,
					const Requirement**									aOutFailedRequirement = NULL);
		bool	CheckAnyList(
					const std::vector<Requirement>&						aRequirements,
					const std::vector<const EntityInstance*>&			aSelves,
					const EntityInstance*								aTarget);
		bool	CheckOpenable(
					const EntityInstance*								aSelf,
					const EntityInstance*								aTarget,
					bool*												aOutInstant);
	}

}