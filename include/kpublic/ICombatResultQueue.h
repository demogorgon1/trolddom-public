#pragma once

namespace kpublic
{

	namespace Components
	{
		struct CombatPublic;
	}

	class ICombatResultQueue
	{
	public:
		virtual				~ICombatResultQueue() {}

		// Virtual interface
		virtual void		AddResourceChange(
								CombatEvent::Id					aCombatEventId,
								uint32_t						aAbilityId,
								uint32_t						aSourceEntityInstanceId,
								uint32_t						aTargetEntityInstanceId,
								Components::CombatPublic*		aCombat,
								size_t							aResourceIndex,
								int32_t							aChange) = 0;
		virtual	void		AddThreatChange(
								uint32_t						aSourceEntityInstanceId,
								uint32_t						aTargetEntityInstanceId,
								int32_t							aThreat) = 0;

	};

}