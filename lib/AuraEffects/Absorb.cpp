#include "../Pcheader.h"

#include <tpublic/AuraEffects/Absorb.h>

namespace tpublic::AuraEffects
{

	int32_t			
	Absorb::FilterDamageInputOnUpdate(
		DirectEffect::DamageType				aDamageType,
		int32_t									aDamage,
		uint32_t&								aCharges,
		int32_t&								aOutAbsorbed) 
	{
		if((m_typeMask == 0 || (m_typeMask & (1 << (uint32_t)aDamageType))) && aDamage > 0 && aCharges > 0)
		{
			int32_t absorbed = Base::Min(aDamage, (int32_t)aCharges);
			aCharges -= (uint32_t)absorbed;
			aOutAbsorbed += absorbed;
			return aDamage - absorbed;
		}

		return aDamage;
	}

}