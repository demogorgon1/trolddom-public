#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/ThreatSource.h>

#include <tpublic/DirectEffects/Heal.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Helpers.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IResourceChangeQueue.h>
#include <tpublic/Resource.h>

namespace tpublic::DirectEffects
{

	void
	Heal::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode*	aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if(aChild->m_name == "function")
					m_function = CombatFunction(aChild);
				else if(aChild->m_name == "max_health_percentage")
					m_maxHealthPercentage = aChild->GetBool();
				else if (aChild->m_name == "conditional_critical_chance_bonus")
					m_conditionalCriticalChanceBonuses.push_back(ConditionalCriticalChanceBonus(aChild));
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
			}
		});
	}

	void	
	Heal::ToStream(
		IWriter*						aStream) const 
	{
		ToStreamBase(aStream);
		m_function.ToStream(aStream);
		aStream->WriteBool(m_maxHealthPercentage);
		aStream->WriteObjects(m_conditionalCriticalChanceBonuses);
	}
			
	bool	
	Heal::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		if(!m_function.FromStream(aStream))
			return false;
		if (!aStream->ReadBool(m_maxHealthPercentage))
			return false;
		if(!aStream->ReadObjects(m_conditionalCriticalChanceBonuses))
			return false;
		return true;
	}

	DirectEffectBase::Result
	Heal::Resolve(
		int32_t							aTick,
		std::mt19937&					aRandom,
		const Manifest*					aManifest,
		CombatEvent::Id					aId,
		uint32_t						aAbilityId,
		const SourceEntityInstance&		aSourceEntityInstance,
		EntityInstance*					aSource,
		EntityInstance*					aTarget,
		const Vec2&						/*aAOETarget*/,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			aResourceChangeQueue,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					aEventQueue,
		const IWorldView*				/*aWorldView*/) 
	{
		const Components::CombatPrivate* sourceCombatPrivate = aSource != NULL ? aSource->GetComponent<Components::CombatPrivate>() : NULL;
		const Components::CombatPublic* targetCombatPublic = aTarget->GetComponent<Components::CombatPublic>();

		if(sourceCombatPrivate == NULL || targetCombatPublic == NULL)
			return Result();

		uint32_t heal = (uint32_t)m_function.EvaluateSourceAndTargetEntityInstances(aRandom, 1.0f, aSource, aTarget);

		if(m_maxHealthPercentage)
		{	
			const Components::CombatPublic::ResourceEntry* targetHealth = targetCombatPublic->GetResourceEntry(Resource::ID_HEALTH);
			
			heal = (targetHealth->m_max * heal) / 100;
		}

		if(heal == 0)
			heal = 1;
				
		CombatEvent::Id result = aId;

		if(m_flags & DirectEffect::FLAG_CAN_BE_CRITICAL && aId == CombatEvent::ID_HIT)
		{
			float critChance = sourceCombatPrivate->m_magicalCriticalStrikeChance + _GetCriticalChanceBonus(aSource->GetComponent<Components::AbilityModifiers>());

			if(Helpers::RandomFloat(aRandom) < critChance / 100.0f)
			{
				heal = (heal * 3) / 2;
				result = CombatEvent::ID_CRITICAL;
			}
		}

		Components::Auras* sourceAuras = aSource->GetComponent<Components::Auras>();
		if (sourceAuras != NULL)
			heal = sourceAuras->FilterHealOutput(aManifest, aSource, aTarget, heal);

		Components::Auras* targetAuras = aTarget->GetComponent<Components::Auras>();
		if(targetAuras != NULL)
			heal = targetAuras->FilterHealInput(aManifest, aSource, aTarget, heal);

		size_t healthResourceIndex;
		if(targetCombatPublic->GetResourceIndex(Resource::ID_HEALTH, healthResourceIndex))
		{
			aResourceChangeQueue->AddResourceChange(
				result,
				DirectEffect::INVALID_DAMAGE_TYPE,
				aAbilityId,
				aSource->GetEntityId(),
				aSource->GetEntityInstanceId(),
				aTarget->GetEntityInstanceId(),
				healthResourceIndex,
				(int32_t)heal,
				0,
				false);

			// Anyone on the target's threat target list should gain threat from this
			const Components::ThreatSource* targetThreatSource = aTarget->GetComponent<Components::ThreatSource>();

			if(targetThreatSource != NULL)
			{
				int32_t threat = (int32_t)heal / 2;
				if(result == CombatEvent::ID_CRITICAL)
					threat = (threat * 3) / 2;

				for(std::unordered_map<uint32_t, int32_t>::const_iterator i = targetThreatSource->m_targets.cbegin(); i != targetThreatSource->m_targets.cend(); i++)
				{
					aEventQueue->EventQueueThreat(aSourceEntityInstance, i->first, threat, aTick);
				}
			}
		}

		return { result };
	}

	bool			
	Heal::CalculateToolTipHeal(
		const EntityInstance*		aEntityInstance,
		const AbilityModifierList*	/*aAbilityModifierList*/,
		uint32_t					/*aAbilityId*/,
		UIntRange&					aOutHeal) const 
	{
		m_function.ToRange(1.0f, aEntityInstance, aOutHeal);
		return true;
	}

	float			
	Heal::_GetCriticalChanceBonus(
		const Components::AbilityModifiers*	aAbilityModifiers) const
	{
		if(m_conditionalCriticalChanceBonuses.empty() || aAbilityModifiers == NULL)
			return 0.0f;

		float bonus = 0.0f;

		for(const ConditionalCriticalChanceBonus& t : m_conditionalCriticalChanceBonuses)
		{
			if(aAbilityModifiers->HasActive(t.m_abilityModifierId))
				bonus += t.m_percent;
		}

		return bonus;
	}

}