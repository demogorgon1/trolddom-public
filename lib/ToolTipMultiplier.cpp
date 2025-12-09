#include "Pcheader.h"

#include <tpublic/Requirements.h>
#include <tpublic/ToolTipMultiplier.h>

namespace tpublic
{

	float	
	ToolTipMultiplier::Resolve(
		const std::vector<ToolTipMultiplier>&	aToolTipMultipliers,
		Type									aType,
		uint32_t								aAbilityId,
		DirectEffect::DamageType				aDamageType,
		const Manifest*							aManifest,
		const EntityInstance*					aEntityInstance)
	{
		float multiplier = 1.0f;

		for(const ToolTipMultiplier& toolTipMultiplier : aToolTipMultipliers)
		{
			if(toolTipMultiplier.m_type != aType)
				continue;

			if(!toolTipMultiplier.m_abilityIds.empty() && Helpers::FindItem(toolTipMultiplier.m_abilityIds, aAbilityId) == SIZE_MAX)
				continue;

			if(toolTipMultiplier.m_typeMask != 0 && (toolTipMultiplier.m_typeMask & (1 << aDamageType)) == 0)
				continue;

			if(!Requirements::CheckList(aManifest, toolTipMultiplier.m_requirements, aEntityInstance, NULL))
				continue;

			multiplier *= toolTipMultiplier.m_multiplier;
		}

		return multiplier;
	}

}