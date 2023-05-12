#pragma once

namespace tpublic
{

	namespace Components
	{
		struct CombatPublic;
	}

	class IResourceChangeQueue
	{
	public:
		virtual				~IResourceChangeQueue() {}

		// Virtual interface
		virtual void		AddResourceChange(
								CombatEvent::Id					aCombatEventId,
								uint32_t						aAbilityId,
								uint32_t						aSourceEntityInstanceId,
								uint32_t						aTargetEntityInstanceId,
								Components::CombatPublic*		aCombat,
								size_t							aResourceIndex,
								int32_t							aChange) = 0;
	};

}