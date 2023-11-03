#pragma once

#include <tpublic/Data/Ability.h>

namespace tpublic
{

	struct Vec2;

	class IAbilityQueue
	{
	public:
		virtual ~IAbilityQueue() {}

		// Virtual interface
		virtual void		AddAbility(
								uint32_t				aSourceEntityInstanceId,
								uint32_t				aTargetEntityInstanceId,
								const Vec2&				aAOETarget,
								const Data::Ability*	aAbility) = 0;
	};

}