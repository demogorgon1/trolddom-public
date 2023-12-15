#pragma once

namespace tpublic
{

	namespace Data
	{
		struct Ability;
	}

	class EntityInstance;

	namespace AbilityRequirements
	{

		bool	Check(
					const Data::Ability*				aAbility,			
					const EntityInstance*				aSelf,
					const EntityInstance*				aTarget);

	}

}