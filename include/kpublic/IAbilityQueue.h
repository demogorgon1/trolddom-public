#pragma once

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
								const Data::Ability*	aAbility) = 0;
	};

}