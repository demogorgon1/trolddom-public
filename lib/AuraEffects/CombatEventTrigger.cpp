#include "../Pcheader.h"

#include <tpublic/AuraEffects/CombatEventTrigger.h>

#include <tpublic/IEventQueue.h>
#include <tpublic/Manifest.h>

namespace tpublic::AuraEffects
{

	void	
	CombatEventTrigger::OnCombatEvent(		
		const Manifest*					aManifest,
		CombatEventType					aType,
		CombatEvent::Id					aCombatEventId,
		uint32_t						aAbilityId,
		SecondaryAbilityCallback		aCallback) const
	{
		if(m_combatEventId == aCombatEventId && m_combatEventType == aType)
		{
			bool shouldTrigger = true;

			if(m_triggerAbilityIds.size() > 0)
			{
				shouldTrigger = false;
				for(uint32_t triggerAbilityId : m_triggerAbilityIds)
				{
					if(triggerAbilityId == aAbilityId)
					{
						shouldTrigger = true;
						break;
					}
				}
			}

			if(shouldTrigger && m_combatEventAbilityMask != 0)
			{
				const Data::Ability* ability = aManifest->GetById<Data::Ability>(aAbilityId);
				if((ability->m_flags & m_combatEventAbilityMask) != m_combatEventAbilityMask)
					shouldTrigger = false;
			}

			if(shouldTrigger)
				aCallback(m_ability, m_probability);
		}
	}

}