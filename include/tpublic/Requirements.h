#pragma once

#include <tpublic/Data/Ability.h>

namespace tpublic
{

	class EntityInstance;

	namespace Requirements
	{

		bool	Check(
					const EntityInstance*				aSelf,
					const EntityInstance*				aTarget,
					const Data::Ability::Requirement*	aRequirement);
		bool	CheckAbility(
					const Data::Ability*				aAbility,			
					const EntityInstance*				aSelf,
					const EntityInstance*				aTarget,
					const Data::Ability::Requirement**	aOutFailedRequirement = NULL);
		bool	CheckOpenable(
					const EntityInstance*				aSelf,
					const EntityInstance*				aTarget,
					bool*								aOutInstant);
	}

}