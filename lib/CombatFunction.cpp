#include "Pcheader.h"

#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/MinionPublic.h>
#include <tpublic/Components/PlayerPublic.h>

#include <tpublic/CombatFunction.h>
#include <tpublic/EntityInstance.h>
#include <tpublic/Helpers.h>
#include <tpublic/IWorldView.h>

namespace tpublic
{

	namespace
	{

		float
		_GetA(
			const Components::CombatPublic*		aCombatPublic,
			const CombatFunction*				aCombatFunction)
		{
			if(aCombatFunction->m_aLevelCurve.IsSet())
				return (float)aCombatFunction->m_aLevelCurve.Sample(aCombatPublic->m_level);
			return aCombatFunction->m_a;
		}

		float
		_GetB(
			const Components::CombatPublic*		aCombatPublic,
			const CombatFunction*				aCombatFunction)
		{
			if(aCombatFunction->m_bLevelCurve.IsSet())
				return (float)aCombatFunction->m_bLevelCurve.Sample(aCombatPublic->m_level);
			return aCombatFunction->m_b;
		}

		float
		_GetC(
			const Components::CombatPublic*		aCombatPublic,
			const CombatFunction*				aCombatFunction)
		{
			if(aCombatFunction->m_cLevelCurve.IsSet())
				return (float)aCombatFunction->m_cLevelCurve.Sample(aCombatPublic->m_level);
			return aCombatFunction->m_c;
		}

		uint32_t
		_SampleUIntRange(	
			CombatFunction::RandomSource		aRandomSource,
			uint32_t							aMin,
			uint32_t							aMax)
		{
			uint32_t value = 0;
			switch (aRandomSource.m_type)
			{
			case CombatFunction::RandomSource::TYPE_RANDOM:
				value = Helpers::RandomInRange<uint32_t>(*aRandomSource.m_random, aMin, aMax);
				break;

			case CombatFunction::RandomSource::TYPE_MIN:
				value = aMin;
				break;

			case CombatFunction::RandomSource::TYPE_MAX:
				value = aMax;
				break;

			default:
				assert(false);
				break;
			}
			return value;
		}

		uint32_t
		_NormalizeWeaponDamage(
			int32_t								aCooldown,
			uint32_t							aDamage)		
		{
			if(aCooldown == 0)
				return 0;

			// Normalize to speed 2.0
			return (aDamage * (uint32_t)20) / (uint32_t)aCooldown;
		}

		const Components::CombatPrivate*
		_GetOwnerCombatPrivate(
			const IWorldView*					aWorldView,	
			const EntityInstance*				aEntityInstance)
		{
			if(aWorldView != NULL)
			{
				const Components::MinionPublic* minionPublic = aEntityInstance->GetComponent<Components::MinionPublic>();
				if (minionPublic != NULL && minionPublic->m_ownerEntityInstanceId != 0)
				{
					const EntityInstance* ownerEntityInstance = aWorldView->WorldViewSingleEntityInstance(minionPublic->m_ownerEntityInstanceId);
					if (ownerEntityInstance != NULL)
						return ownerEntityInstance->GetComponent<Components::CombatPrivate>();
				}
			}
			return NULL;
		}

		float
		_GetInput(
			const Manifest*						/*aManifest*/,
			const IWorldView*					aWorldView,
			CombatFunction::RandomSource		aRandomSource,
			const EntityInstance*				aEntityInstance,
			const Components::CombatPublic*		aCombatPublic,
			const Components::CombatPrivate*	aCombatPrivate,
			CombatFunction::Input				aInput,
			bool								aIsOffHandAttack)
		{
			switch(aInput)
			{
			case CombatFunction::INPUT_PVP_CONTROL_POINTS:				
				{
					if (aWorldView == NULL || aCombatPublic == NULL || aCombatPublic->m_factionId == 0)
						return 0.0f;

					return (float)aWorldView->WorldViewGetPVPFactionControlPointCount(aCombatPublic->m_factionId);
				}
				break;

			case CombatFunction::INPUT_HEALTH_CURRENT:	
				if(aCombatPublic != NULL)
					return (float)aCombatPublic->GetResource(Resource::ID_HEALTH);
				break;

			case CombatFunction::INPUT_HEALTH_MAX:
				if(aCombatPublic != NULL)
					return (float)aCombatPublic->GetResourceMax(Resource::ID_HEALTH);
				break;

			case CombatFunction::INPUT_MANA_CURRENT:
				if(aCombatPublic != NULL)
					return (float)aCombatPublic->GetResource(Resource::ID_MANA);
				break;

			case CombatFunction::INPUT_MANA_MAX:
				if(aCombatPublic != NULL)
					return (float)aCombatPublic->GetResourceMax(Resource::ID_MANA);
				break;

			case CombatFunction::INPUT_LEVEL:
				if (aCombatPublic != NULL)
					return (float)aCombatPublic->m_level;
				break;

			case CombatFunction::INPUT_ATTACK_POWER:
				if (aCombatPrivate != NULL)
					return (float)aCombatPrivate->m_attackPower;
				break;

			case CombatFunction::INPUT_MANA_BASE:
				if (aCombatPrivate != NULL)
					return (float)aCombatPrivate->m_baseMana;
				break;

			case CombatFunction::INPUT_SPELL_DAMAGE:
				if (aCombatPrivate != NULL)
					return (float)aCombatPrivate->m_spellDamage;
				break;

			case CombatFunction::INPUT_BLOCK_VALUE:
				if(aCombatPrivate != NULL)
					return (float)aCombatPrivate->m_blockValue;
				break;

			case CombatFunction::INPUT_STRENGTH:
				if (aEntityInstance != NULL && aEntityInstance->IsPlayer())
				{
					const Components::PlayerPublic* playerPublic = aEntityInstance->GetComponent<Components::PlayerPublic>();
					return playerPublic->m_stats.m_stats[Stat::ID_STRENGTH];
				}
				break;

			case CombatFunction::INPUT_OWNER_SPELL_DAMAGE:
				if(aEntityInstance != NULL)
				{
					const Components::CombatPrivate* ownerCombatPrivate = _GetOwnerCombatPrivate(aWorldView, aEntityInstance);
					if(ownerCombatPrivate != NULL)
						return (float)ownerCombatPrivate->m_spellDamage;
					else
						return 0.0f;
				}
				break;

			case CombatFunction::INPUT_OWNER_HEALING:
				if(aEntityInstance != NULL)
				{
					const Components::CombatPrivate* ownerCombatPrivate = _GetOwnerCombatPrivate(aWorldView, aEntityInstance);
					if(ownerCombatPrivate != NULL)
						return (float)ownerCombatPrivate->m_healing;
					else
						return 0.0f;
				}
				break;

			case CombatFunction::INPUT_HEALING:
				if (aCombatPrivate != NULL)
					return (float)aCombatPrivate->m_healing;
				break;

			case CombatFunction::INPUT_MINION_WEAPON:
				if(aEntityInstance != NULL)
				{
					const Components::MinionPublic* minionPublic = aEntityInstance->GetComponent<Components::MinionPublic>();
					if(minionPublic != NULL && minionPublic->m_toolTipWeaponDamageBase.has_value()) 
						return (float)_SampleUIntRange(aRandomSource, minionPublic->m_toolTipWeaponDamageBase->m_min, minionPublic->m_toolTipWeaponDamageBase->m_max); // FIXME: don't do this on server
					else if(aCombatPrivate != NULL)
						return (float)_SampleUIntRange(aRandomSource, aCombatPrivate->m_weaponDamageRangeMin, aCombatPrivate->m_weaponDamageRangeMax);
				}
				break;

			case CombatFunction::INPUT_WEAPON:
				if(aCombatPublic != NULL && aCombatPublic->m_overrideWeaponDamageRangeMin != 0 && aCombatPublic->m_overrideWeaponDamageRangeMax != 0)
					return (float)_SampleUIntRange(aRandomSource, aCombatPublic->m_overrideWeaponDamageRangeMin, aCombatPublic->m_overrideWeaponDamageRangeMax);

				if(aCombatPrivate != NULL)
				{
					if(aIsOffHandAttack)
						return (float)_SampleUIntRange(aRandomSource, aCombatPrivate->m_offHandDamageRangeMin, aCombatPrivate->m_offHandDamageRangeMax);
					else
						return (float)_SampleUIntRange(aRandomSource, aCombatPrivate->m_weaponDamageRangeMin, aCombatPrivate->m_weaponDamageRangeMax);
				}
				break;

			case CombatFunction::INPUT_WEAPON_AVERAGE:
				if (aCombatPublic != NULL && aCombatPublic->m_overrideWeaponDamageRangeMin != 0 && aCombatPublic->m_overrideWeaponDamageRangeMax != 0)
					return (float)(aCombatPublic->m_overrideWeaponDamageRangeMin + aCombatPublic->m_overrideWeaponDamageRangeMax) * 0.5f;

				if (aCombatPrivate != NULL)
				{
					if(aIsOffHandAttack)
						return (float)(aCombatPrivate->m_offHandDamageRangeMin + aCombatPrivate->m_offHandDamageRangeMax) * 0.5f;
					else
						return (float)(aCombatPrivate->m_weaponDamageRangeMin + aCombatPrivate->m_weaponDamageRangeMax) * 0.5f;
				}
				break;

			case CombatFunction::INPUT_RANGED:
				if(aCombatPrivate != NULL)
					return (float)_SampleUIntRange(aRandomSource, aCombatPrivate->m_rangedDamageRangeMin, aCombatPrivate->m_rangedDamageRangeMax);
				break;

			case CombatFunction::INPUT_RANGED_AVERAGE:
				if (aCombatPrivate != NULL)
					return (float)(aCombatPrivate->m_rangedDamageRangeMin + aCombatPrivate->m_rangedDamageRangeMax) * 0.5f; 
				break;

			case CombatFunction::INPUT_WEAPON_NORMALIZED:
				if (aCombatPrivate != NULL && aCombatPublic != NULL && aCombatPublic->m_overrideWeaponDamageRangeMin != 0 && aCombatPublic->m_overrideWeaponDamageRangeMax != 0)
					return (float)_NormalizeWeaponDamage(aCombatPrivate->m_weaponCooldown, _SampleUIntRange(aRandomSource, aCombatPublic->m_overrideWeaponDamageRangeMin, aCombatPublic->m_overrideWeaponDamageRangeMax));

				if(aCombatPrivate != NULL)
				{
					if(aIsOffHandAttack)
						return (float)_NormalizeWeaponDamage(aCombatPrivate->m_weaponCooldown, _SampleUIntRange(aRandomSource, aCombatPrivate->m_offHandDamageRangeMin, aCombatPrivate->m_offHandDamageRangeMax));
					else
						return (float)_NormalizeWeaponDamage(aCombatPrivate->m_weaponCooldown, _SampleUIntRange(aRandomSource, aCombatPrivate->m_weaponDamageRangeMin, aCombatPrivate->m_weaponDamageRangeMax));
				}
				break;

			case CombatFunction::INPUT_WEAPON_AVERAGE_NORMALIZED:
				if (aCombatPrivate != NULL && aCombatPublic != NULL && aCombatPublic->m_overrideWeaponDamageRangeMin != 0 && aCombatPublic->m_overrideWeaponDamageRangeMax != 0)
					return (float)_NormalizeWeaponDamage(aCombatPrivate->m_weaponCooldown, (aCombatPublic->m_overrideWeaponDamageRangeMin + aCombatPublic->m_overrideWeaponDamageRangeMax) / 2);

				if (aCombatPrivate != NULL)
				{
					if(aIsOffHandAttack)
						return (float)_NormalizeWeaponDamage(aCombatPrivate->m_weaponCooldown, (aCombatPrivate->m_offHandDamageRangeMin + aCombatPrivate->m_offHandDamageRangeMax) / 2);
					else
						return (float)_NormalizeWeaponDamage(aCombatPrivate->m_weaponCooldown, (aCombatPrivate->m_weaponDamageRangeMin + aCombatPrivate->m_weaponDamageRangeMax) / 2);
				}
				break;

			default:
				break;
			}
			TP_CHECK(false, "Invalid function input.");
			return 0.0f;
		}
	}

	//--------------------------------------------------------------------------

	float		
	CombatFunction::Evaluate(
		const Manifest*						aManifest,
		const IWorldView*					aWorldView,
		CombatFunction::RandomSource		aRandomSource,
		float								aMultiplier,
		const EntityInstance*				aEntityInstance,
		const Components::CombatPublic*		aCombatPublic,
		const Components::CombatPrivate*	aCombatPrivate,
		bool								aIsOffHandAttack) const
	{
		float output = 0.0f;

		switch(m_expression)
		{
		case EXPRESSION_A:
			output = _GetA(aCombatPublic, this);
			break;

		case EXPRESSION_A_MUL_X:
			output = _GetA(aCombatPublic, this) * _GetInput(aManifest, aWorldView, aRandomSource, aEntityInstance, aCombatPublic, aCombatPrivate, m_x, aIsOffHandAttack);
			break;

		case EXPRESSION_A_MUL_X_PLUS_B:
			output = _GetA(aCombatPublic, this) * _GetInput(aManifest, aWorldView, aRandomSource, aEntityInstance, aCombatPublic, aCombatPrivate, m_x, aIsOffHandAttack) + _GetB(aCombatPublic, this);
			break;

		case EXPRESSION_A_MUL_X_PLUS_B_MUL_Y:
			output = _GetA(aCombatPublic, this) * _GetInput(aManifest, aWorldView, aRandomSource, aEntityInstance, aCombatPublic, aCombatPrivate, m_x, aIsOffHandAttack) +
				_GetB(aCombatPublic, this) * _GetInput(aManifest, aWorldView, aRandomSource, aEntityInstance, aCombatPublic, aCombatPrivate, m_y, aIsOffHandAttack);
			break;

		case EXPRESSION_A_MUL_X_PLUS_B_MUL_Y_PLUS_C:
			output = _GetA(aCombatPublic, this) * _GetInput(aManifest, aWorldView, aRandomSource, aEntityInstance, aCombatPublic, aCombatPrivate, m_x, aIsOffHandAttack) +
				_GetB(aCombatPublic, this) * _GetInput(aManifest, aWorldView, aRandomSource, aEntityInstance, aCombatPublic, aCombatPrivate, m_y, aIsOffHandAttack) +
				_GetC(aCombatPublic, this);
			break;

		case EXPRESSION_X_PLUS_A:
			output = _GetInput(aManifest, aWorldView, aRandomSource, aEntityInstance, aCombatPublic, aCombatPrivate, m_x, aIsOffHandAttack) + _GetA(aCombatPublic, this);
			break;

		case EXPRESSION_X:
			output = _GetInput(aManifest, aWorldView, aRandomSource, aEntityInstance, aCombatPublic, aCombatPrivate, m_x, aIsOffHandAttack);
			break;

		default:
			break;
		}

		if(aCombatPublic->IsElite())
			output *= m_eliteMultiplier;

		if(aWorldView->WorldViewIsHeroic())
			output *= m_heroicMultiplier;

		if(m_spread > 0.0f)
		{
			switch (aRandomSource.m_type)
			{
			case RandomSource::TYPE_RANDOM:
				output = output - m_spread + 2.0f * m_spread * Helpers::RandomFloat(*aRandomSource.m_random);
				break;

			case RandomSource::TYPE_MIN:
				output = output - m_spread;
				break;

			case RandomSource::TYPE_MAX:
				output = output + m_spread;
				break;

			default:
				assert(false);
				break;
			}
		}

		return output * aMultiplier;
	}

	float		
	CombatFunction::EvaluateEntityInstance(
		const Manifest*						aManifest,
		const IWorldView*					aWorldView,
		CombatFunction::RandomSource		aRandomSource,
		float								aMultiplier,
		const EntityInstance*				aEntityInstance,
		bool								aIsOffHandAttack) const
	{
		return Evaluate(
			aManifest,
			aWorldView,
			aRandomSource, 
			aMultiplier,
			aEntityInstance,
			aEntityInstance->GetComponent<Components::CombatPublic>(),
			aEntityInstance->GetComponent<Components::CombatPrivate>(),
			aIsOffHandAttack);
	}

	float		
	CombatFunction::EvaluateSourceAndTargetEntityInstances(
		const Manifest*						aManifest,
		const IWorldView*					aWorldView,
		RandomSource						aRandomSource,
		float								aMultiplier,
		const EntityInstance*				aSourceEntityInstance,
		const EntityInstance*				aTargetEntityInstance,
		bool								aIsOffHandAttack) const
	{
		const EntityInstance* entityInstance = NULL;
		switch(m_entity)
		{
		case ENTITY_SOURCE: entityInstance = aSourceEntityInstance; break;
		case ENTITY_TARGET: entityInstance = aTargetEntityInstance; break;
		default:			return 0.0f;
		}
		return EvaluateEntityInstance(aManifest, aWorldView, aRandomSource, aMultiplier, entityInstance, aIsOffHandAttack);
	}

	void		
	CombatFunction::ToRange(
		const Manifest*						aManifest,
		const IWorldView*					aWorldView,
		float								aMultiplier,
		const EntityInstance*				aEntityInstance,
		bool								aIsOffHandAttack,
		UIntRange&							aOut) const
	{
		aOut.m_min = (uint32_t)EvaluateEntityInstance(aManifest, aWorldView, RandomSource(RandomSource::TYPE_MIN), aMultiplier, aEntityInstance, aIsOffHandAttack);
		aOut.m_max = (uint32_t)EvaluateEntityInstance(aManifest, aWorldView, RandomSource(RandomSource::TYPE_MAX), aMultiplier, aEntityInstance, aIsOffHandAttack);
	}

}