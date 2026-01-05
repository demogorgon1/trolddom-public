#include "../Pcheader.h"

#include <tpublic/AuraEffects/CombatEventTrigger.h>

#include <tpublic/Data/Ability.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/Manifest.h>

namespace tpublic::AuraEffects
{

	void
	CombatEventTrigger::FromSource(
		const SourceNode* aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if (!FromSourceBase(aChild))
			{
				if (aChild->m_name == "combat_event")
				{
					TP_VERIFY(aChild->m_annotation, aChild->m_debugInfo, "Missing combat event type annotation.");
					m_combatEventType = AuraEffectBase::SourceToCombatEventType(aChild->m_annotation.get());

					if (aChild->m_type == SourceNode::TYPE_ARRAY)
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
				else if (aChild->m_name == "ability")
				{
					m_ability = SecondaryAbility(aChild);
				}
				else if (aChild->m_name == "trigger_abilities")
				{
					aChild->GetIdArray(DataType::ID_ABILITY, m_triggerAbilityIds);
				}
				else if (aChild->m_name == "combat_event_ability_mask")
				{
					m_combatEventAbilityMask = Data::Ability::GetFlags(aChild, NULL);
				}
				else if (aChild->m_name == "combat_event_ability_reject_mask")
				{
					m_combatEventAbilityRejectMask = Data::Ability::GetFlags(aChild, NULL);
				}
				else if (aChild->m_name == "probability")
				{
					m_probability = aChild->GetUInt32();
				}
				else if(aChild->m_name == "ability_retain_source")
				{
					m_abilityRetainSource = aChild->GetBool();
				}
				else if (aChild->m_name == "ability_aura_source")
				{
					m_abilityAuraSource = aChild->GetBool();
				}
				else
				{
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				}
			}
		});
	}

	void
	CombatEventTrigger::ToStream(
		IWriter* aStream) const 
	{
		ToStreamBase(aStream);

		aStream->WritePODs(m_combatEventIds);
		aStream->WritePOD(m_combatEventType);
		m_ability.ToStream(aStream);
		aStream->WriteUInts(m_triggerAbilityIds);
		aStream->WriteUInt(m_combatEventAbilityMask);
		aStream->WriteUInt(m_combatEventAbilityRejectMask);
		aStream->WriteUInt(m_probability);
		aStream->WriteBool(m_abilityRetainSource);
		aStream->WriteBool(m_abilityAuraSource);
	}

	bool
	CombatEventTrigger::FromStream(
		IReader* aStream) 
	{
		if (!FromStreamBase(aStream))
			return false;

		if (!aStream->ReadPODs(m_combatEventIds))
			return false;
		if (!aStream->ReadPOD(m_combatEventType))
			return false;
		if (!m_ability.FromStream(aStream))
			return false;
		if (!aStream->ReadUInts(m_triggerAbilityIds))
			return false;
		if (!aStream->ReadUInt(m_combatEventAbilityMask))
			return false;
		if (!aStream->ReadUInt(m_combatEventAbilityRejectMask))
			return false;
		if (!aStream->ReadUInt(m_probability))
			return false;
		if (!aStream->ReadBool(m_abilityRetainSource))
			return false;
		if (!aStream->ReadBool(m_abilityAuraSource))
			return false;
		return true;
	}

	AuraEffectBase*
	CombatEventTrigger::Copy() const 
	{
		CombatEventTrigger* t = new CombatEventTrigger();
		t->CopyBase(this);
		t->m_combatEventIds = m_combatEventIds;
		t->m_combatEventType = m_combatEventType;
		t->m_ability = m_ability;
		t->m_triggerAbilityIds = m_triggerAbilityIds;
		t->m_combatEventAbilityMask = m_combatEventAbilityMask;
		t->m_combatEventAbilityRejectMask = m_combatEventAbilityRejectMask;
		t->m_probability = m_probability;
		t->m_abilityRetainSource = m_abilityRetainSource;
		t->m_abilityAuraSource = m_abilityAuraSource;
		return t;
	}

	void	
	CombatEventTrigger::OnCombatEvent(		
		const Manifest*					aManifest,
		uint32_t						/*aAuraId*/,
		CombatEventType					aType,
		CombatEvent::Id					aCombatEventId,
		uint32_t						aAbilityId,
		const EntityInstance*			aAuraSourceEntityInstance,
		const EntityInstance*			aSourceEntityInstance,
		const EntityInstance*			aTargetEntityInstance,
		std::mt19937*					aRandom,
		IEventQueue*					aEventQueue) const
	{
		if(ShouldTriggerOnCombatEvent(aCombatEventId) && m_combatEventType == aType)
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

			if(shouldTrigger && (m_combatEventAbilityMask != 0 || m_combatEventAbilityRejectMask != 0))
			{
				const Data::Ability* ability = aManifest->GetById<Data::Ability>(aAbilityId);

				if (shouldTrigger && m_combatEventAbilityMask != 0)
				{
					if ((ability->m_flags & m_combatEventAbilityMask) != m_combatEventAbilityMask)
						shouldTrigger = false;
				}

				if (shouldTrigger && m_combatEventAbilityRejectMask != 0 )	
				{
					if (ability->m_flags & m_combatEventAbilityRejectMask)
						shouldTrigger = false;
				}
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

					if(m_abilityRetainSource)
						source = aSourceEntityInstance;
					else if(m_abilityAuraSource)
						source = aAuraSourceEntityInstance;

					if(source != NULL)
						aEventQueue->EventQueueAbility({ source->GetEntityInstanceId(), source->GetSeq() }, target->GetEntityInstanceId(), Vec2(), ability, ItemInstanceReference(), NULL);
				}
			}
		}
	}

}