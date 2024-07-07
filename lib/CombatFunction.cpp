#include "Pcheader.h"

#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>

#include <tpublic/CombatFunction.h>
#include <tpublic/EntityInstance.h>
#include <tpublic/Helpers.h>

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

		float
		_GetInput(
			CombatFunction::RandomSource		aRandomSource,
			const Components::CombatPublic*		aCombatPublic,
			const Components::CombatPrivate*	aCombatPrivate,
			CombatFunction::Input				aInput)
		{
			switch(aInput)
			{
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

			case CombatFunction::INPUT_MANA_BASE:
				if (aCombatPrivate != NULL)
					return (float)aCombatPrivate->m_baseMana;
				break;

			case CombatFunction::INPUT_SPELL_DAMAGE:
				if (aCombatPrivate != NULL)
					return (float)aCombatPrivate->m_spellDamage;
				break;

			case CombatFunction::INPUT_HEALING:
				if (aCombatPrivate != NULL)
					return (float)aCombatPrivate->m_healing;
				break;

			case CombatFunction::INPUT_WEAPON:
				if(aCombatPrivate != NULL)
					return (float)_SampleUIntRange(aRandomSource, aCombatPrivate->m_weaponDamageRangeMin, aCombatPrivate->m_weaponDamageRangeMax);
				break;

			case CombatFunction::INPUT_WEAPON_AVERAGE:
				if (aCombatPrivate != NULL)
					return (float)(aCombatPrivate->m_weaponDamageRangeMin + aCombatPrivate->m_weaponDamageRangeMax) * 0.5f; 
				break;

			case CombatFunction::INPUT_RANGED:
				if(aCombatPrivate != NULL)
					return (float)_SampleUIntRange(aRandomSource, aCombatPrivate->m_rangedDamageRangeMin, aCombatPrivate->m_rangedDamageRangeMax);
				break;

			case CombatFunction::INPUT_RANGED_AVERAGE:
				if (aCombatPrivate != NULL)
					return (float)(aCombatPrivate->m_rangedDamageRangeMin + aCombatPrivate->m_rangedDamageRangeMax) * 0.5f; 
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
		CombatFunction::RandomSource		aRandomSource,
		const Components::CombatPublic*		aCombatPublic,
		const Components::CombatPrivate*	aCombatPrivate) const
	{
		float output = 0.0f;

		switch(m_expression)
		{
		case EXPRESSION_A:
			output = _GetA(aCombatPublic, this);
			break;

		case EXPRESSION_A_MUL_X:
			output = _GetA(aCombatPublic, this) * _GetInput(aRandomSource, aCombatPublic, aCombatPrivate, m_x);
			break;

		case EXPRESSION_A_MUL_X_PLUS_B:
			output = _GetA(aCombatPublic, this) * _GetInput(aRandomSource, aCombatPublic, aCombatPrivate, m_x) + _GetB(aCombatPublic, this);
			break;

		case EXPRESSION_A_MUL_X_PLUS_B_MUL_Y:
			output = _GetA(aCombatPublic, this) * _GetInput(aRandomSource, aCombatPublic, aCombatPrivate, m_x) + _GetB(aCombatPublic, this) * _GetInput(aRandomSource, aCombatPublic, aCombatPrivate, m_y);
			break;

		case EXPRESSION_X_PLUS_A:
			output = _GetInput(aRandomSource, aCombatPublic, aCombatPrivate, m_x) + _GetA(aCombatPublic, this);
			break;

		case EXPRESSION_X:
			output = _GetInput(aRandomSource, aCombatPublic, aCombatPrivate, m_x);
			break;

		default:
			break;
		}

		if(aCombatPublic->IsElite())
			output *= m_eliteMultiplier;

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

		return output;
	}

	float		
	CombatFunction::EvaluateEntityInstance(
		CombatFunction::RandomSource		aRandomSource,
		const EntityInstance*				aEntityInstance) const
	{
		return Evaluate(
			aRandomSource, 
			aEntityInstance->GetComponent<Components::CombatPublic>(),
			aEntityInstance->GetComponent<Components::CombatPrivate>());
	}

	void		
	CombatFunction::ToRange(
		const EntityInstance*				aEntityInstance,
		UIntRange&							aOut) const
	{
		aOut.m_min = (uint32_t)EvaluateEntityInstance(RandomSource(RandomSource::TYPE_MIN), aEntityInstance);
		aOut.m_max = (uint32_t)EvaluateEntityInstance(RandomSource(RandomSource::TYPE_MAX), aEntityInstance);
	}

}