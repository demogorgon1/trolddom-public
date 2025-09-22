#include "../Pcheader.h"

#include <tpublic/AuraEffects/DamageOutputModifier.h>

#include <tpublic/Helpers.h>
#include <tpublic/Requirements.h>

namespace tpublic::AuraEffects
{

	int32_t
	DamageOutputModifier::FilterDamageOutput(
		const Manifest*				/*aManifest*/,
		const EntityInstance*		/*aSource*/,
		const EntityInstance*		/*aTarget*/,
		DirectEffect::DamageType	aDamageType,
		int32_t						aDamage,
		uint32_t					aAbilityId) const 
	{
		if (!m_applyToAbilityIds.empty() && Helpers::FindItem(m_applyToAbilityIds, aAbilityId) == SIZE_MAX)
			return aDamage;

		int32_t damage = aDamage;

		if (m_typeMask & (1 << (uint32_t)aDamageType))
		{
			damage *= m_multiplierNumerator;
			damage /= m_multiplierDenominator;
		}

		return damage;
	}

}