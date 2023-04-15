#pragma once

namespace kaos_public
{

	class ICombatEventQueue
	{
	public:
		virtual ~ICombatEventQueue() {}

		// Virtual interface
		virtual void		AddCombatEvent(
								uint32_t				aSourceEntityInstanceId,
								uint32_t				aTargetEntityInstanceId,
								const Data::Ability*	aAbility) = 0;
	};

}