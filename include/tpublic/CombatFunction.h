#pragma once

#include "SourceNode.h"

namespace tpublic
{

	namespace Components
	{
		struct CombatPrivate;
		struct CombatPublic;
	}

	class EntityInstance;

	class CombatFunction
	{
	public:
		enum Expression : uint8_t
		{
			INVALID_EXPRESSION,

			EXPRESSION_A,
			EXPRESSION_A_MUL_X,
			EXPRESSION_A_MUL_X_PLUS_B
		};

		enum Input : uint8_t
		{
			INVALID_INPUT,

			INPUT_HEALTH_CURRENT,
			INPUT_HEALTH_MAX,
			INPUT_MANA_CURRENT,
			INPUT_MANA_MAX,
			INPUT_LEVEL
		};

		static Expression
		SourceToExpression(
			const SourceNode*					aSource)
		{
			std::string_view t(aSource->GetIdentifier());
			if(t == "a")
				return EXPRESSION_A;
			else if(t == "a_mul_x")
				return EXPRESSION_A_MUL_X;
			else if (t == "a_mul_x_plus_b")
				return EXPRESSION_A_MUL_X_PLUS_B;
			TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid expression.", aSource->GetIdentifier());
			return INVALID_EXPRESSION;
		}

		static Input
		SourceToInput(
			const SourceNode*					aSource)
		{
			std::string_view t(aSource->GetIdentifier());
			if (t == "health_current")
				return INPUT_HEALTH_CURRENT;
			else if (t == "health_max")
				return INPUT_HEALTH_MAX;
			else if (t == "mana_current")
				return INPUT_HEALTH_CURRENT;
			else if (t == "mana_max")
				return INPUT_HEALTH_MAX;
			else if (t == "level")
				return INPUT_LEVEL;
			TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid input.", aSource->GetIdentifier());
			return INVALID_INPUT;
		}

		CombatFunction()
		{

		}

		CombatFunction(
			const SourceNode*					aSource)
		{
			if(aSource->m_type == SourceNode::TYPE_NUMBER)
			{
				m_expression = EXPRESSION_A;
				m_a = aSource->GetFloat();
			}
			else
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "expression")
						m_expression = SourceToExpression(aChild);
					else if(aChild->m_name == "x")
						m_x = SourceToInput(aChild);
					else if(aChild->m_name == "a")
						m_a = aChild->GetFloat();
					else if (aChild->m_name == "b")
						m_b = aChild->GetFloat();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}
		}
		
		void
		ToStream(
			IWriter*							aWriter) const
		{
			aWriter->WritePOD(m_expression);
			aWriter->WritePOD(m_x);
			aWriter->WriteFloat(m_a);
			aWriter->WriteFloat(m_b);
		}

		bool
		FromStream(
			IReader*							aReader)
		{
			if (!aReader->ReadPOD(m_expression))
				return false;
			if (!aReader->ReadPOD(m_x))
				return false;
			if (!aReader->ReadFloat(m_a))
				return false;
			if (!aReader->ReadFloat(m_b))
				return false;
			return true;
		}

		bool 
		IsSet() const
		{
			return m_expression != INVALID_EXPRESSION;
		}

		float		Evaluate(
						const Components::CombatPublic*		aCombatPublic,
						const Components::CombatPrivate*	aCombatPrivate) const;
		float		EvaluateEntityInstance(
						const EntityInstance*				aEntityInstance) const;
	
		// Public data
		Expression		m_expression = INVALID_EXPRESSION;
		Input			m_x = INVALID_INPUT;
		float			m_a = 0.0f;
		float			m_b = 0.0f;
	};
}