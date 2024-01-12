#pragma once

#include "EntityState.h"
#include "SimpleDirectEffect.h"

namespace tpublic
{

	namespace Components
	{
		struct CombatPublic;
	}

	class IResourceChangeQueue
	{
	public:
		typedef std::function<void()> UpdateCallback;

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
		virtual void		AddUpdateCallback(
								UpdateCallback					aUpdateCallback) = 0;
		virtual void		AddSimpleDirectEffect(
								uint32_t						aSourceEntityInstanceId,
								uint32_t						aTargetEntityInstanceId,
								SimpleDirectEffect::Id			aSimpleDirectEffectId) = 0;

	};

}