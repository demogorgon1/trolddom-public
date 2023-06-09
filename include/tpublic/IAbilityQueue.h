#pragma once

#include <tpublic/Data/Ability.h>

namespace tpublic
{

	class IAbilityQueue
	{
	public:
		virtual ~IAbilityQueue() {}

		// Virtual interface
		virtual void		AddAbility(
								uint32_t				aSourceEntityInstanceId,
								uint32_t				aTargetEntityInstanceId,
								const tpublic::Vec2&	aAOETarget,
								const Data::Ability*	aAbility) = 0;
	};

}