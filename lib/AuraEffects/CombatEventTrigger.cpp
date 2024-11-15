#include "../Pcheader.h"

#include <tpublic/Data/Ability.h>

#include <tpublic/AuraEffects/CombatEventTrigger.h>

#include <tpublic/IEventQueue.h>
#include <tpublic/Manifest.h>

namespace tpublic::AuraEffects
{

	void	
	CombatEventTrigger::OnCombatEvent(		
		CombatEventType					aType,
		CombatEvent::Id					aCombatEventId,
		uint32_t						aAbilityId,
		SecondaryAbilityCallback		aCallback) const
	{
		if(m_combatEventId == aCombatEventId && m_combatEventType == aType)
		{
			if(m_triggerAbilityIds.size() > 0)
			{
				bool shouldTrigger = false;
				for(uint32_t triggerAbilityId : m_triggerAbilityIds)
				{
					if(triggerAbilityId == aAbilityId)
					{
						shouldTrigger = true;
						break;
					}
				}

				if(!shouldTrigger)
					return;
			}

			aCallback(m_ability);
		}
	}

}