#include "../Pcheader.h"

#include <tpublic/Data/Ability.h>

#include <tpublic/AuraEffects/CombatEventTrigger.h>

#include <tpublic/IEventQueue.h>
#include <tpublic/Manifest.h>

namespace tpublic::AuraEffects
{

	void	
	CombatEventTrigger::OnCombatEvent(		
		CombatEvent::Id					aCombatEventId,
		SecondaryAbilityCallback		aCallback) const
	{
		if(m_combatEventId == aCombatEventId)
			aCallback(m_ability);
	}

}