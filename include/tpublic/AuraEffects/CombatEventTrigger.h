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

			// AuraEffectBase implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild) 
				{
					if(!FromSourceBase(aChild))
					{
						if(aChild->m_name == "combat_event")
						{
							TP_VERIFY(aChild->m_annotation, aChild->m_debugInfo, "Missing combat event type annotation.");
							m_combatEventType = AuraEffectBase::SourceToCombatEventType(aChild->m_annotation.get());
							m_combatEventId = CombatEvent::StringToId(aChild->GetIdentifier());
							TP_VERIFY(m_combatEventId != CombatEvent::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid combat event.", aChild->GetIdentifier());
						}
						else if(aChild->m_name == "ability")
						{
							m_ability = SecondaryAbility(aChild);
						}
						else if(aChild->m_name == "trigger_abilities")
						{
							aChild->GetIdArray(DataType::ID_ABILITY, m_triggerAbilityIds);
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);

				aStream->WritePOD(m_combatEventId);
				aStream->WritePOD(m_combatEventType);
				m_ability.ToStream(aStream);
				aStream->WriteUInts(m_triggerAbilityIds);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;

				if (!aStream->ReadPOD(m_combatEventId))
					return false;
				if (!aStream->ReadPOD(m_combatEventType))
					return false;
				if (!m_ability.FromStream(aStream))
					return false;
				if(!aStream->ReadUInts(m_triggerAbilityIds))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				CombatEventTrigger* t = new CombatEventTrigger();
				t->CopyBase(this);
				t->m_combatEventId = m_combatEventId;
				t->m_combatEventType = m_combatEventType;
				t->m_ability = m_ability;
				t->m_triggerAbilityIds = m_triggerAbilityIds;
				return t;
			}

			void	OnCombatEvent(						
						CombatEventType					aType,
						CombatEvent::Id					aCombatEventId,
						uint32_t						aAbilityId,
						SecondaryAbilityCallback		aCallback) const override;

			// Public data
			AuraEffectBase::CombatEventType	m_combatEventType = AuraEffectBase::INVALID_COMBAT_EVENT_TYPE;
			CombatEvent::Id					m_combatEventId = CombatEvent::ID_HIT;
			SecondaryAbility				m_ability;
			std::vector<uint32_t>			m_triggerAbilityIds;
		};

	}

}