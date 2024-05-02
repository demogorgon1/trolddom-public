#include "../Pcheader.h"

#include <tpublic/AuraEffects/CastModifier.h>

#include <tpublic/Manifest.h>

namespace tpublic::AuraEffects
{

	bool				
	CastModifier::UpdateCastTime(
		const Manifest*					aManifest,
		uint32_t						aAbilityId,
		uint32_t&						aCharges,
		int32_t&						aCastTime) 
	{
		const Data::Ability* ability = aManifest->GetById<Data::Ability>(aAbilityId);
		if((ability->m_flags & m_abilityFlags) != m_abilityFlags)
			return false;

		if(m_useCharge)
		{
			if(aCharges == 0)
				return false;

			aCharges--;
		}

		aCastTime = (int32_t)((float)aCastTime * m_castTimeModifier);
			
		return true;
	}

}