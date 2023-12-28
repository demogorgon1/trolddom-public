#pragma once

#include "Requirement.h"

namespace tpublic
{

	class EntityInstance;

	namespace Requirements
	{

		bool	Check(
					const EntityInstance*				aSelf,
					const EntityInstance*				aTarget,
					const Requirement*					aRequirement);
		bool	CheckList(
					const std::vector<Requirement>&		aRequirements,
					const EntityInstance*				aSelf,
					const EntityInstance*				aTarget,
					const Requirement**					aOutFailedRequirement = NULL);
		bool	CheckOpenable(
					const EntityInstance*				aSelf,
					const EntityInstance*				aTarget,
					bool*								aOutInstant);
	}

}