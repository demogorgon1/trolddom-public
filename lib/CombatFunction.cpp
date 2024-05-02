#include "Pcheader.h"

#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>

#include <tpublic/CombatFunction.h>
#include <tpublic/EntityInstance.h>

namespace tpublic
{

	namespace
	{

		float
		_GetInput(
			const Components::CombatPublic*		aCombatPublic,
			const Components::CombatPrivate*	/*aCombatPrivate*/,
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
		const Components::CombatPublic*		aCombatPublic,
		const Components::CombatPrivate*	aCombatPrivate) const
	{
		switch(m_expression)
		{
		case EXPRESSION_A:
			return m_a;

		case EXPRESSION_A_MUL_X:
			return m_a * _GetInput(aCombatPublic, aCombatPrivate, m_x);

		case EXPRESSION_A_MUL_X_PLUS_B:
			return m_a * _GetInput(aCombatPublic, aCombatPrivate, m_x) + m_b;

		default:
			break;
		}

		return 0.0f;
	}

	float		
	CombatFunction::EvaluateEntityInstance(
		const EntityInstance*				aEntityInstance) const
	{
		return Evaluate(
			aEntityInstance->GetComponent<Components::CombatPublic>(),
			aEntityInstance->GetComponent<Components::CombatPrivate>());
	}

}