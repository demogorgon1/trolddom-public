#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/PlayerPrivate.h>

#include <tpublic/DirectEffects/Damage.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Helpers.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IResourceChangeQueue.h>

namespace tpublic::DirectEffects
{

	void
	Damage::FromSource(
		const SourceNode*			aSource)
	{
		aSource->ForEachChild([&](
			const SourceNode*	aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if(aChild->m_name == "damage_type")
				{
					m_damageType = DirectEffect::StringToDamageType(aChild->GetIdentifier());
				}
				else if(aChild->m_name == "conditional_critical_chance_bonus")
				{
					m_conditionalCriticalChanceBonuses.push_back(ConditionalCriticalChanceBonus(ConditionalCriticalChanceBonus::TYPE_ABILITY_MODIFIER, aChild));
				}
				else if (aChild->m_name == "aura_conditional_critical_chance_bonus")
				{
					m_conditionalCriticalChanceBonuses.push_back(ConditionalCriticalChanceBonus(ConditionalCriticalChanceBonus::TYPE_AURA, aChild));
				}
				else if(aChild->m_name == "function")
				{
					m_function = CombatFunction(aChild);
				}
				else if(aChild->m_name == "ability_modifier_multipliers")
				{
					aChild->GetObject()->ForEachChild([&](
						const SourceNode* aAbilityModifierMultiplier)
					{
						uint32_t abilityModifierId = aAbilityModifierMultiplier->m_sourceContext->m_persistentIdTable->GetId(aAbilityModifierMultiplier->m_debugInfo, DataType::ID_ABILITY_MODIFIER, aAbilityModifierMultiplier->m_name.c_str());
						m_abilityModifierMultipliers[abilityModifierId] = aAbilityModifierMultiplier->GetFloat();
					});
				}
				else
				{
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				}
			}
		});
	}

	void	
	Damage::ToStream(
		IWriter*					aStream) const
	{
		ToStreamBase(aStream);
		aStream->WritePOD(m_damageType);
		aStream->WriteObjects(m_conditionalCriticalChanceBonuses);
		m_function.ToStream(aStream);

		aStream->WriteUInt(m_abilityModifierMultipliers.size());
		for(std::unordered_map<uint32_t, float>::const_iterator i = m_abilityModifierMultipliers.cbegin(); i != m_abilityModifierMultipliers.cend(); i++)
		{
			aStream->WriteUInt(i->first);
			aStream->WriteFloat(i->second);
		}
	}
			
	bool	
	Damage::FromStream(
		IReader*					aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		if (!aStream->ReadPOD(m_damageType))
			return false;
		if(!aStream->ReadObjects(m_conditionalCriticalChanceBonuses))
			return false;
		if(!m_function.FromStream(aStream))
			return false;

		{
			size_t count = 0;
			if(!aStream->ReadUInt(count))
				return false;

			for(size_t i = 0; i < count; i++)
			{
				uint32_t abilityModifierId = 0;
				if(!aStream->ReadUInt(abilityModifierId))
					return false;

				float multiplier = 0.0f;
				if(!aStream->ReadFloat(multiplier))
					return false;

				m_abilityModifierMultipliers[abilityModifierId] = multiplier;
			}
		}

		return true;
	}

	DirectEffectBase::Result
	Damage::Resolve(
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
		const IWorldView*				aWorldView) 
	{
		if(aSource == NULL)
			return Result();

		if(m_flags & DirectEffect::FLAG_SELF)
			aTarget = aSource;

		const Components::CombatPrivate* sourceCombatPrivate = aSource->GetComponent<Components::CombatPrivate>();
		const Components::CombatPrivate* targetCombatPrivate = aTarget->GetComponent<Components::CombatPrivate>();
		Components::CombatPublic* sourceCombatPublic = aSource->GetComponent<Components::CombatPublic>();
		Components::CombatPublic* targetCombatPublic = aTarget->GetComponent<Components::CombatPublic>();
		Components::Auras* targetAuras = aTarget->GetComponent<Components::Auras>();
		const Components::Auras* sourceAuras = aSource->GetComponent<Components::Auras>();
		const Components::AbilityModifiers* abilityModifiers = aSource->GetComponent<Components::AbilityModifiers>();

		if(targetCombatPublic == NULL)
			return Result();

		uint32_t damage = (uint32_t)m_function.EvaluateSourceAndTargetEntityInstances(aManifest, aWorldView, aRandom, _GetDamageModifier(abilityModifiers), aSource, aTarget);

		if(damage == 0)
			damage = 1;

		CombatEvent::Id result = aId;

		if(m_flags & DirectEffect::FLAG_CAN_BE_CRITICAL && aId == CombatEvent::ID_HIT && sourceCombatPrivate != NULL)
		{
			float chance = 0.0f;

			if(m_flags & DirectEffect::FLAG_IS_MAGICAL)
				chance = sourceCombatPrivate->m_magicalCriticalStrikeChance;
			else
				chance = sourceCombatPrivate->m_physicalCriticalStrikeChance;

			chance += _GetCriticalChanceBonus(sourceAuras, abilityModifiers);

			if(Helpers::RandomFloat(aRandom) < chance / 100.0f)
			{
				damage = (damage * 3) / 2;

				result = CombatEvent::ID_CRITICAL;
			}
		}

		const Components::PlayerPrivate* playerPrivate = aSource->GetEntityId() == 0 ? aSource->GetComponent<Components::PlayerPrivate>() : NULL;

		DirectEffect::DamageType damageType = _GetDamageType(aSource, playerPrivate != NULL ? playerPrivate->m_abilityModifierList : NULL, aAbilityId);

		if(damageType == DirectEffect::DAMAGE_TYPE_PHYSICAL && targetCombatPrivate != NULL)
		{
			// Damage reduction from armor
			float damageMultiplier = 1.0f - (float)targetCombatPrivate->m_armor / (float)(targetCombatPrivate->m_armor + 400 + 85 * sourceCombatPublic->m_level);
			damage = (uint32_t)((float)damage * damageMultiplier);
		}

		if (sourceAuras != NULL)
			damage = sourceAuras->FilterDamageOutput(aManifest, aSource, aTarget, damageType, damage);

		if (targetAuras != NULL)
			damage = targetAuras->FilterDamageInput(aManifest, aSource, aTarget, damageType, damage);

		uint32_t blocked = 0;

		if(result == CombatEvent::ID_BLOCK && targetCombatPrivate != NULL)
		{
			blocked = targetCombatPrivate->m_blockValue;
			if(blocked > damage)
				blocked = damage;

			damage -= blocked;			
		}

		// Rage generation for source
		if(m_flags & DirectEffect::FLAG_GENERATE_RAGE)
		{
			size_t rageResourceIndex;
			if (sourceCombatPublic->GetResourceIndex(Resource::ID_RAGE, rageResourceIndex))
			{
				const AbilityMetrics* abilityMetrics = &aManifest->m_abilityMetrics;
				int32_t rageConstant = (int32_t)abilityMetrics->m_rageConstantAtLevelCurve.Sample(sourceCombatPublic->m_level) + 1;
				int32_t rageBasePerSecond = result == CombatEvent::ID_CRITICAL ? (int32_t)abilityMetrics->m_rageCritBasePerSecond : (int32_t)abilityMetrics->m_rageHitBasePerSecond;
				int32_t rageBase = (sourceCombatPrivate->m_weaponCooldown * rageBasePerSecond) / 20;
				int32_t rage = (int32_t)damage / rageConstant + rageBase;
				int32_t rageMax = (4 * (int32_t)damage) / rageConstant;
				if(rage > rageMax)
					rage = rageMax;

				aResourceChangeQueue->AddResourceChange(
					result,
					damageType,
					aAbilityId,
					aSource->GetEntityId(),
					aSource->GetEntityInstanceId(),
					aSource->GetEntityInstanceId(),
					rageResourceIndex,
					rage,
					0,
					false);
			}
		}

		// Rage geneation for target
		{
			size_t rageResourceIndex;
			if (targetCombatPublic->GetResourceIndex(Resource::ID_RAGE, rageResourceIndex))
			{
				const AbilityMetrics* abilityMetrics = &aManifest->m_abilityMetrics;
				int32_t rageConstant = (int32_t)abilityMetrics->m_rageConstantAtLevelCurve.Sample(targetCombatPublic->m_level) + 1;
				int32_t rage = (int32_t)(5 * damage) / (8 * rageConstant) + 1;

				aResourceChangeQueue->AddResourceChange(
					result,
					damageType,
					aAbilityId,
					aSource->GetEntityId(),
					aSource->GetEntityInstanceId(),
					aTarget->GetEntityInstanceId(),
					rageResourceIndex,
					rage,
					0,
					false);
			}
		}

		// Health
		size_t healthResourceIndex;
		if(targetCombatPublic->GetResourceIndex(Resource::ID_HEALTH, healthResourceIndex))
		{
			if(targetAuras != NULL)
				targetAuras->OnDamageInput(aManifest, aSource, aTarget, m_damageType, (int32_t)damage, result, aEventQueue, aWorldView, aResourceChangeQueue);

			aResourceChangeQueue->AddResourceChange(
				result,
				damageType,
				aAbilityId,
				aSource->GetEntityId(),
				aSource->GetEntityInstanceId(),
				aTarget->GetEntityInstanceId(),
				healthResourceIndex,
				-(int32_t)damage,
				blocked,
				false);

			if(m_flags & DirectEffect::FLAG_LEECH)
			{
				size_t sourceHealthResourceIndex;
				if (sourceCombatPublic->GetResourceIndex(Resource::ID_HEALTH, sourceHealthResourceIndex))
				{
					aResourceChangeQueue->AddResourceChange(
						result,
						DirectEffect::INVALID_DAMAGE_TYPE,
						aAbilityId,
						aTarget->GetEntityId(),
						aTarget->GetEntityInstanceId(),
						aSource->GetEntityInstanceId(),
						healthResourceIndex,
						(int32_t)damage / 2, // FIXME: might want this to not always be 50%
						0,
						false);
				}
			}
		}

		// Threat
		{
			int32_t threat = (int32_t)damage;
			if (result == CombatEvent::ID_CRITICAL)
				threat = (threat * 3) / 2; // Crits generate more threat per damage than non-crits

			if (aTarget->GetEntityId() != 0) // Not a player
				aEventQueue->EventQueueThreat(aSourceEntityInstance, aTarget->GetEntityInstanceId(), threat, aTick);
		}

		return { result };
	}

	bool			
	Damage::CalculateToolTipDamage(
		const EntityInstance*				aEntityInstance,
		const AbilityModifierList*			aAbilityModifierList,
		uint32_t							aAbilityId,
		UIntRange&							aOutDamage,
		DirectEffect::DamageType&			aOutDamageType)	const
	{
		float damageModifier = 1.0f;

		if(aAbilityModifierList != NULL)
		{
			const std::vector<const Data::AbilityModifier*>* modifierList = aAbilityModifierList->GetAbility(aAbilityId);
			if(modifierList != NULL)
			{
				for (const Data::AbilityModifier* abilityModifier : *modifierList)
				{
					std::unordered_map<uint32_t, float>::const_iterator i = m_abilityModifierMultipliers.find(abilityModifier->m_id);
					if (i != m_abilityModifierMultipliers.cend())
						damageModifier *= i->second;
				}
			}
		}

		m_function.ToRange(NULL, NULL, damageModifier, aEntityInstance, aOutDamage);
		aOutDamageType = _GetDamageType(aEntityInstance, aAbilityModifierList, aAbilityId);
		return true;
	}

	DirectEffect::DamageType	
	Damage::_GetDamageType(
		const EntityInstance*				/*aEntityInstance*/,		
		const AbilityModifierList*			aAbilityModifierList,
		uint32_t							aAbilityId) const
	{
		DirectEffect::DamageType damageType = m_damageType;

		if(aAbilityModifierList != NULL && aAbilityId != 0)
		{
			DirectEffect::DamageType modifyDamageType = aAbilityModifierList->GetAbilityModifyDamageType(aAbilityId);
			if (modifyDamageType != DirectEffect::INVALID_DAMAGE_TYPE)
				damageType = modifyDamageType;
		}

		return damageType;
	}

	float			
	Damage::_GetCriticalChanceBonus(
		const Components::Auras*			aAuras,
		const Components::AbilityModifiers*	aAbilityModifiers) const
	{
		if(m_conditionalCriticalChanceBonuses.empty() || aAbilityModifiers == NULL)
			return 0.0f;

		float bonus = 0.0f;

		for(const ConditionalCriticalChanceBonus& t : m_conditionalCriticalChanceBonuses)
		{
			switch(t.m_type)
			{
			case ConditionalCriticalChanceBonus::TYPE_ABILITY_MODIFIER:
				if (aAbilityModifiers->HasActive(t.m_id))
					bonus += t.m_percent;
				break;

			case ConditionalCriticalChanceBonus::TYPE_AURA:
				if(aAuras->HasAura(t.m_id))
					bonus += t.m_percent;
				break;

			default:
				break;
			}
		}

		return bonus;
	}

	float						
	Damage::_GetDamageModifier(
		const Components::AbilityModifiers* aAbilityModifiers) const
	{
		float modifier = 1.0f;

		if(aAbilityModifiers != NULL)
		{
			for (uint32_t abilityModifierId : aAbilityModifiers->m_active)
			{
				std::unordered_map<uint32_t, float>::const_iterator i = m_abilityModifierMultipliers.find(abilityModifierId);
				if (i != m_abilityModifierMultipliers.cend())
					modifier *= i->second;
			}
		}

		return modifier;
	}

}