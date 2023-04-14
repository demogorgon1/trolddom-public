#pragma once

namespace kaos_public
{

	class ICombatEventQueue
	{
	public:
		virtual ~ICombatEventQueue() {}

		// Virtual interface
		virtual void		AddCombatLogEvent(
								uint32_t				aSourceEntityInstanceId,
								uint32_t				aTargetEntityInstanceId,
								const Data::Ability*	aAbility) = 0;
	};

}