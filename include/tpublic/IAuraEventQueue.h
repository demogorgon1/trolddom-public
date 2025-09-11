#pragma once

#include "AuraEffectBase.h"

namespace tpublic
{

	class IAuraEventQueue
	{
	public:
		virtual				~IAuraEventQueue() {}

		// Virtual interface
		virtual void		ApplyAura(
								uint32_t										aAbilityId,
								uint32_t										aAuraId,
								const SourceEntityInstance&						aSourceEntityInstance,
								bool											aSourceIsPlayerOrMinion,
								uint32_t										aTargetEntityInstanceId,
								int32_t											aAuraDurationModifier,
								std::vector<std::unique_ptr<AuraEffectBase>>&	aAuraEffects) = 0;
	};

}