#pragma once

#include "../AuraEffectBase.h"
#include "../SecondaryAbility.h"

namespace tpublic
{

	namespace AuraEffects
	{

		struct CombatEventTrigger
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_COMBAT_EVENT_TRIGGER;

			CombatEventTrigger()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~CombatEventTrigger()
			{

			}

			bool
			ShouldTriggerOnCombatEvent(
				CombatEvent::Id			aCombatEventId) const
			{
				if(m_combatEventIds.empty() && aCombatEventId == CombatEvent::ID_HIT)
					return true; // Defaults to triggering on hits

				for(CombatEvent::Id t : m_combatEventIds)
				{
					if(t == aCombatEventId)
						return true;
				}
				return false;
			}

			// AuraEffectBase implementation
			void				FromSource(
									const SourceNode*				aSource) override;
			void				ToStream(
									IWriter*						aStream) const override;
			bool				FromStream(
									IReader*						aStream) override;
			AuraEffectBase*		Copy() const override;
			void				OnCombatEvent(						
									const Manifest*					aManifest,
									uint32_t						aAuraId,
									CombatEventType					aType,
									CombatEvent::Id					aCombatEventId,
									uint32_t						aAbilityId,
									const EntityInstance*			aSourceEntityInstance,
									const EntityInstance*			aTargetEntityInstance,
									std::mt19937*					aRandom,
									IEventQueue*					aEventQueue) const override;

			// Public data
			AuraEffectBase::CombatEventType	m_combatEventType = AuraEffectBase::INVALID_COMBAT_EVENT_TYPE;
			std::vector<CombatEvent::Id>	m_combatEventIds;
			uint32_t						m_combatEventAbilityMask = 0;
			uint32_t						m_combatEventAbilityRejectMask = 0;
			SecondaryAbility				m_ability;
			bool							m_abilityRetainSource = false;
			std::vector<uint32_t>			m_triggerAbilityIds;
			uint32_t						m_probability = 0;
		};

	}

}