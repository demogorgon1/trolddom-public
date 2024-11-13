#include "Pcheader.h"

#include <tpublic/AuraEffects/AbilityOnFade.h>
#include <tpublic/AuraEffects/Absorb.h>
#include <tpublic/AuraEffects/CastModifier.h>
#include <tpublic/AuraEffects/CombatEventTrigger.h>
#include <tpublic/AuraEffects/DamageInputEffect.h>
#include <tpublic/AuraEffects/DamageInputModifier.h>
#include <tpublic/AuraEffects/DamageOutputModifier.h>
#include <tpublic/AuraEffects/HealInputModifier.h>
#include <tpublic/AuraEffects/HealOutputModifier.h>
#include <tpublic/AuraEffects/Immobilize.h>
#include <tpublic/AuraEffects/Immortality.h>
#include <tpublic/AuraEffects/MoveSpeedModifier.h>
#include <tpublic/AuraEffects/Repeat.h>
#include <tpublic/AuraEffects/Stun.h>
#include <tpublic/AuraEffects/Taunt.h>
#include <tpublic/AuraEffects/ThreatModifier.h>

#include <tpublic/AuraEffectFactory.h>

namespace tpublic
{

	AuraEffectFactory::AuraEffectFactory()
	{
		_Register<AuraEffects::AbilityOnFade>();
		_Register<AuraEffects::Absorb>();
		_Register<AuraEffects::CastModifier>();
		_Register<AuraEffects::CombatEventTrigger>();
		_Register<AuraEffects::DamageInputEffect>();
		_Register<AuraEffects::DamageInputModifier>();
		_Register<AuraEffects::DamageOutputModifier>();
		_Register<AuraEffects::HealInputModifier>();
		_Register<AuraEffects::HealOutputModifier>();
		_Register<AuraEffects::Immobilize>();
		_Register<AuraEffects::Immortality>();
		_Register<AuraEffects::MoveSpeedModifier>();
		_Register<AuraEffects::Repeat>();
		_Register<AuraEffects::Stun>();
		_Register<AuraEffects::Taunt>();
		_Register<AuraEffects::ThreatModifier>();
	}

	AuraEffectFactory::~AuraEffectFactory()
	{

	}

	AuraEffectBase*
	AuraEffectFactory::Create(
		uint32_t			aId) const
	{
		if(!m_functions[aId])
			return NULL;

		return m_functions[aId]();
	}

}
