#include "../Pcheader.h"

#include <tpublic/AuraEffects/CombatEventTrigger.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/Manifest.h>

namespace tpublic::AuraEffects
{

	void	
	CombatEventTrigger::OnCombatEvent(		
		const Manifest*					aManifest,
		uint32_t						/*aAuraId*/,
		CombatEventType					aType,
		CombatEvent::Id					aCombatEventId,
		uint32_t						aAbilityId,
		const EntityInstance*			aSourceEntityInstance,
		const EntityInstance*			aTargetEntityInstance,
		std::mt19937*					aRandom,
		IEventQueue*					aEventQueue) const
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

			if(shouldTrigger && m_probability > 0 && !Helpers::RandomRoll(*aRandom, m_probability))
				shouldTrigger = false;

			if(shouldTrigger)
			{
				const EntityInstance* target = m_ability.ResolveTarget(aSourceEntityInstance, aTargetEntityInstance);
				if (target != NULL)
				{
					const Data::Ability* ability = aManifest->GetById<Data::Ability>(m_ability.m_abilityId);

					const EntityInstance* source = aType == COMBAT_EVENT_TYPE_TARGET ? aTargetEntityInstance : aSourceEntityInstance;

					aEventQueue->EventQueueAbility({ source->GetEntityInstanceId(), source->GetSeq() }, target->GetEntityInstanceId(), Vec2(), ability, ItemInstanceReference(), NULL);
				}
			}
		}
	}

}