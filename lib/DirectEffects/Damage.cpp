#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>

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
				else if(aChild->m_name == "level_curve")
				{
					m_levelCurve = UIntCurve<uint32_t>(aChild);
				}
				else if(aChild->m_name == "base")
				{
					if(aChild->m_type == SourceNode::TYPE_ARRAY)
					{
						m_damageBase = DirectEffect::DAMAGE_BASE_RANGE;

						if(aChild->m_children.size() == 1)
						{
							m_damageBaseRangeMin = aChild->m_children[0]->GetUInt32();
							m_damageBaseRangeMax = m_damageBaseRangeMin;
						}
						else if (aChild->m_children.size() == 2)
						{
							m_damageBaseRangeMin = aChild->m_children[0]->GetUInt32();
							m_damageBaseRangeMax = aChild->m_children[1]->GetUInt32();
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid damage base definition.", aChild->m_name.c_str());
						}
					}
					else if(aChild->m_type == SourceNode::TYPE_IDENTIFIER && aChild->m_value == "weapon")
					{
						m_damageBase = DirectEffect::DAMAGE_BASE_WEAPON;
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "Not a valid damage base.", aChild->m_name.c_str());
					}
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
		aStream->WritePOD(m_damageBase);
		m_levelCurve.ToStream(aStream);

		if(m_damageBase == DirectEffect::DAMAGE_BASE_RANGE)
		{
			aStream->WriteUInt(m_damageBaseRangeMin);
			aStream->WriteUInt(m_damageBaseRangeMax);
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
		if (!aStream->ReadPOD(m_damageBase))
			return false;
		if(!m_levelCurve.FromStream(aStream))
			return false;

		if (m_damageBase == DirectEffect::DAMAGE_BASE_RANGE)
		{
			if (!aStream->ReadUInt(m_damageBaseRangeMin))
				return false;
			if (!aStream->ReadUInt(m_damageBaseRangeMax))
				return false;
		}

		return true;
	}

	void
	Damage::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					aRandom,
		const Manifest*					aManifest,
		CombatEvent::Id					aId,
		uint32_t						aAbilityId,
		EntityInstance*					aSource,
		EntityInstance*					aTarget,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			aResourceChangeQueue,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					aEventQueue,
		const IWorldView*				/*aWorldView*/) 
	{
		const Components::CombatPrivate* sourceCombatPrivate = aSource->GetComponent<Components::CombatPrivate>();
		const Components::CombatPrivate* targetCombatPrivate = aTarget->GetComponent<Components::CombatPrivate>();
		Components::CombatPublic* sourceCombatPublic = aSource->GetComponent<Components::CombatPublic>();
		Components::CombatPublic* targetCombatPublic = aTarget->GetComponent<Components::CombatPublic>();
		const Components::Auras* targetAuras = aTarget->GetComponent<Components::Auras>();

		if(targetCombatPublic == NULL)
			return;

		uint32_t damage = 0;

		switch(m_damageBase)
		{
		case DirectEffect::DAMAGE_BASE_RANGE:		
			damage = Helpers::RandomInRange(aRandom, m_damageBaseRangeMin, m_damageBaseRangeMax); 
			break;

		case DirectEffect::DAMAGE_BASE_WEAPON:
			TP_CHECK(sourceCombatPrivate != NULL, "No weapon damage available.");
			damage = Helpers::RandomInRange(aRandom, sourceCombatPrivate->m_weaponDamageRangeMin, sourceCombatPrivate->m_weaponDamageRangeMax);
			break;

		default:
			break;
		}

		damage += m_levelCurve.Sample(sourceCombatPublic->m_level);

		CombatEvent::Id result = aId;

		if(m_flags & DirectEffect::FLAG_CAN_BE_CRITICAL && aId == CombatEvent::ID_HIT && sourceCombatPrivate != NULL)
		{
			float chance = 0.0f;

			if(m_flags & DirectEffect::FLAG_IS_MAGICAL)
				chance = sourceCombatPrivate->m_magicalCriticalStrikeChance;
			else
				chance = sourceCombatPrivate->m_physicalCriticalStrikeChance;

			if(Helpers::RandomFloat(aRandom) < chance / 100.0f)
			{
				damage = (damage * 3) / 2;

				result = CombatEvent::ID_CRITICAL;
			}
		}

		if((m_flags & DirectEffect::FLAG_IS_MAGICAL) == 0)
		{
			// Damage reduction from armor
			float damageMultiplier = 1.0f - (float)targetCombatPrivate->m_armor / (float)(targetCombatPrivate->m_armor + 400 + 85 * sourceCombatPublic->m_level);
			damage = (uint32_t)((float)damage * damageMultiplier);
		}

		if (targetAuras != NULL)
			damage = targetAuras->FilterDamageInput(m_damageType, damage);

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
					aAbilityId,
					aSource->GetEntityInstanceId(),
					aSource->GetEntityInstanceId(),
					sourceCombatPublic,
					rageResourceIndex,
					rage);
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
					aAbilityId,
					aSource->GetEntityInstanceId(),
					aTarget->GetEntityInstanceId(),
					targetCombatPublic,
					rageResourceIndex,
					rage);
			}
		}

		// Health
		size_t healthResourceIndex;
		if(targetCombatPublic->GetResourceIndex(Resource::ID_HEALTH, healthResourceIndex))
		{
			aResourceChangeQueue->AddResourceChange(
				result,
				aAbilityId,
				aSource->GetEntityInstanceId(),
				aTarget->GetEntityInstanceId(),
				targetCombatPublic,
				healthResourceIndex,
				-(int32_t)damage);
		}

		// Threat
		{
			int32_t threat = (int32_t)damage;
			if (result == CombatEvent::ID_CRITICAL)
				threat = (threat * 3) / 2; // Crits generate more threat per damage than non-crits

			if (aTarget->GetEntityId() != 0) // Not a player
				aEventQueue->EventQueueThreat(aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId(), threat);
		}
	}


}