#pragma once

#include "AuraEffects/CombatEventTrigger.h"
#include "AuraEffects/DamageInputModifier.h"
#include "AuraEffects/DamageOutputModifier.h"
#include "AuraEffects/HealInputModifier.h"
#include "AuraEffects/HealOutputModifier.h"
#include "AuraEffects/Immortality.h"
#include "AuraEffects/MoveSpeedModifier.h"
#include "AuraEffects/Repeat.h"
#include "AuraEffects/Stun.h"
#include "AuraEffects/Taunt.h"
#include "AuraEffects/ThreatModifier.h"

#include "AuraEffect.h"

namespace tpublic
{

	class AuraEffectFactory
	{
	public:
		AuraEffectFactory()
		{
			_Register<AuraEffects::CombatEventTrigger>();
			_Register<AuraEffects::DamageInputModifier>();
			_Register<AuraEffects::DamageOutputModifier>();
			_Register<AuraEffects::HealInputModifier>();
			_Register<AuraEffects::HealOutputModifier>();
			_Register<AuraEffects::Immortality>();
			_Register<AuraEffects::MoveSpeedModifier>();
			_Register<AuraEffects::Repeat>();
			_Register<AuraEffects::Stun>();
			_Register<AuraEffects::Taunt>();
			_Register<AuraEffects::ThreatModifier>();
		}

		~AuraEffectFactory()
		{

		}

		AuraEffectBase*
		Create(
			uint32_t			aId) const
		{
			if(!m_functions[aId])
				return NULL;

			return m_functions[aId]();
		}

	private:

		std::function<AuraEffectBase*()>			m_functions[AuraEffect::NUM_IDS];

		template<typename _T>
		void
		_Register()
		{
			m_functions[_T::ID] = []() { return new _T(); };
		}

	};

}
