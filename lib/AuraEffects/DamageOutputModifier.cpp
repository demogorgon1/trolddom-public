#include "../Pcheader.h"

#include <tpublic/AuraEffects/DamageOutputModifier.h>

#include <tpublic/Requirements.h>

namespace tpublic::AuraEffects
{

	int32_t
	DamageOutputModifier::FilterDamageOutput(
		const Manifest*				aManifest,
		const EntityInstance*		aSource,
		const EntityInstance*		aTarget,
		DirectEffect::DamageType	aDamageType,
		int32_t						aDamage) const 
	{
		int32_t damage = aDamage;

		if (m_typeMask & (1 << (uint32_t)aDamageType))
		{
			if (Requirements::CheckList(aManifest, m_requirements, aSource, aTarget))
			{
				damage *= m_multiplierNumerator;
				damage /= m_multiplierDenominator;
			}
		}

		return damage;
	}

}