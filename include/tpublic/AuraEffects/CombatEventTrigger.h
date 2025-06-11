#pragma once

#include <tpublic/Data/Ability.h>

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

							if(aChild->m_type == SourceNode::TYPE_ARRAY)
							{
								aChild->ForEachChild([&](
									const SourceNode* aItem)
								{
									CombatEvent::Id combatEventId = CombatEvent::StringToId(aItem->GetIdentifier());
									TP_VERIFY(combatEventId != CombatEvent::INVALID_ID, aItem->m_debugInfo, "'%s' is not a valid combat event.", aItem->GetIdentifier());
									m_combatEventIds.push_back(combatEventId);
								});
							}
							else
							{
								m_combatEventIds = { CombatEvent::StringToId(aChild->GetIdentifier()) };
								TP_VERIFY(m_combatEventIds[0] != CombatEvent::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid combat event.", aChild->GetIdentifier());
							}
						}
						else if(aChild->m_name == "ability")
						{
							m_ability = SecondaryAbility(aChild);
						}
						else if(aChild->m_name == "trigger_abilities")
						{
							aChild->GetIdArray(DataType::ID_ABILITY, m_triggerAbilityIds);
						}
						else if(aChild->m_name == "combat_event_ability_mask")
						{
							m_combatEventAbilityMask = Data::Ability::GetFlags(aChild, NULL);
						}
						else if (aChild->m_name == "probability")
						{
							m_probability = aChild->GetUInt32();
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

				aStream->WritePODs(m_combatEventIds);
				aStream->WritePOD(m_combatEventType);
				m_ability.ToStream(aStream);
				aStream->WriteUInts(m_triggerAbilityIds);
				aStream->WriteUInt(m_combatEventAbilityMask);
				aStream->WriteUInt(m_probability);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;

				if (!aStream->ReadPODs(m_combatEventIds))
					return false;
				if (!aStream->ReadPOD(m_combatEventType))
					return false;
				if (!m_ability.FromStream(aStream))
					return false;
				if(!aStream->ReadUInts(m_triggerAbilityIds))
					return false;
				if (!aStream->ReadUInt(m_combatEventAbilityMask))
					return false;
				if (!aStream->ReadUInt(m_probability))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				CombatEventTrigger* t = new CombatEventTrigger();
				t->CopyBase(this);
				t->m_combatEventIds = m_combatEventIds;
				t->m_combatEventType = m_combatEventType;
				t->m_ability = m_ability;
				t->m_triggerAbilityIds = m_triggerAbilityIds;
				t->m_combatEventAbilityMask = m_combatEventAbilityMask;
				t->m_probability = m_probability;
				return t;
			}

			void	OnCombatEvent(						
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
			SecondaryAbility				m_ability;
			std::vector<uint32_t>			m_triggerAbilityIds;
			uint32_t						m_probability = 0;
		};

	}

}