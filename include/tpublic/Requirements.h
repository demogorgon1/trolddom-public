#pragma once

namespace tpublic
{

	namespace Data
	{
		struct Ability;
	}

	class EntityInstance;

	namespace Requirements
	{

		bool	CheckAbility(
					const Data::Ability*				aAbility,			
					const EntityInstance*				aSelf,
					const EntityInstance*				aTarget);
		bool	CheckOpenable(
					const EntityInstance*				aSelf,
					const EntityInstance*				aTarget,
					bool*								aOutInstant);
	}

}