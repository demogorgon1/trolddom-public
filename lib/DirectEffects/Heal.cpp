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
		return true;
	}

	DirectEffectBase::Result
	Heal::Resolve(
		int32_t							aTick,
		std::mt19937&					aRandom,
		const Manifest*					/*aManifest*/,
		CombatEvent::Id					aId,
		uint32_t						aAbilityId,
		EntityInstance*					aSource,
		EntityInstance*					aTarget,
		const Vec2&						/*aAOETarget*/,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			aResourceChangeQueue,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					aEventQueue,
		const IWorldView*				/*aWorldView*/) 
	{
		const Components::CombatPrivate* sourceCombatPrivate = aSource->GetComponent<Components::CombatPrivate>();
		Components::CombatPublic* targetCombatPublic = aTarget->GetComponent<Components::CombatPublic>();

		if(sourceCombatPrivate == NULL || targetCombatPublic == NULL)
			return Result();

		uint32_t heal = (uint32_t)m_function.EvaluateEntityInstance(aRandom, aSource);

		if(m_maxHealthPercentage)
		{	
			const Components::CombatPublic::ResourceEntry* targetHealth = targetCombatPublic->GetResourceEntry(Resource::ID_HEALTH);
			
			heal = (targetHealth->m_max * heal) / 100;
		}
				
		CombatEvent::Id result = aId;

		if(m_flags & DirectEffect::FLAG_CAN_BE_CRITICAL && aId == CombatEvent::ID_HIT)
		{
			float chance = (float)sourceCombatPrivate->m_magicalCriticalStrikeChance / (float)UINT32_MAX;

			if(Helpers::RandomFloat(aRandom) < chance)
			{
				heal = (heal * 3) / 2;
				result = CombatEvent::ID_CRITICAL;
			}
		}

		Components::Auras* sourceAuras = aSource->GetComponent<Components::Auras>();
		if (sourceAuras != NULL)
			heal = sourceAuras->FilterHealOutput(heal);

		Components::Auras* targetAuras = aTarget->GetComponent<Components::Auras>();
		if(targetAuras != NULL)
			heal = targetAuras->FilterHealInput(heal);

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
				targetCombatPublic,
				NULL,
				healthResourceIndex,
				(int32_t)heal,
				0,
				false);

			// Anyone on the target's threat target list should gain threat from this
			const Components::ThreatSource* targetThreatSource = aTarget->GetComponent<Components::ThreatSource>();

			if(targetThreatSource != NULL)
			{
				int32_t threat = (int32_t)heal;
				if(result == CombatEvent::ID_CRITICAL)
					threat = (threat * 3) / 2;

				for(std::unordered_map<uint32_t, int32_t>::const_iterator i = targetThreatSource->m_targets.cbegin(); i != targetThreatSource->m_targets.cend(); i++)
				{
					aEventQueue->EventQueueThreat(aSource->GetEntityInstanceId(), i->first, threat, aTick);
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
		m_function.ToRange(aEntityInstance, aOutHeal);
		return true;
	}

}