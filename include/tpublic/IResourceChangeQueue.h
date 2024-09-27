#pragma once

#include "EntityState.h"
#include "SimpleDirectEffect.h"

namespace tpublic
{

	namespace Components
	{
		struct Auras;
		struct CombatPublic;
	}

	class SourceEntityInstance;

	class IResourceChangeQueue
	{
	public:
		typedef std::function<void()> UpdateCallback;

		virtual				~IResourceChangeQueue() {}

		// Virtual interface
		virtual void		AddResourceChange(
								CombatEvent::Id							aCombatEventId,
								tpublic::DirectEffect::DamageType		aDamageType,
								uint32_t								aAbilityId,
								uint32_t								aSourceEntityId,
								uint32_t								aSourceEntityInstanceId,
								uint32_t								aTargetEntityInstanceId,
								Components::CombatPublic*				aCombat,
								Components::Auras*						aAuras,
								size_t									aResourceIndex,
								int32_t									aChange,
								uint32_t								aBlocked,
								bool									aSilent) = 0;
		virtual void		AddUpdateCallback(
								UpdateCallback							aUpdateCallback) = 0;
		virtual void		AddSimpleDirectEffect(
								const SourceEntityInstance&				aSourceEntityInstance,
								uint32_t								aTargetEntityInstanceId,
								SimpleDirectEffect::Id					aSimpleDirectEffectId,
								uint32_t								aParam) = 0;

	};

}