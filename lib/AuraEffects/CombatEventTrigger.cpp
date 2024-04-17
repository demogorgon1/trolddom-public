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
		SecondaryAbilityCallback		aCallback) const
	{
		if(m_combatEventId == aCombatEventId && m_combatEventType == aType)
			aCallback(m_ability);
	}

}