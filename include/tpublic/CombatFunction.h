#pragma once

#include "SourceNode.h"
#include "UIntCurve.h"

namespace tpublic
{

	namespace Components
	{
		struct CombatPrivate;
		struct CombatPublic;
	}

	class EntityInstance;
	class UIntRange;

	class CombatFunction
	{
	public:
		struct RandomSource
		{
			enum Type
			{
				TYPE_RANDOM,
				TYPE_MIN,
				TYPE_MAX
			};

			RandomSource(
				Type							aType = TYPE_RANDOM,
				std::mt19937*					aRandom = NULL)
				: m_type(aType)
				, m_random(aRandom)
			{

			}

			RandomSource(
				std::mt19937&					aRandom)
				: m_type(TYPE_RANDOM)
				, m_random(&aRandom)
			{

			}

			Type				m_type;
			std::mt19937*		m_random;
		};

		enum Expression : uint8_t
		{
			INVALID_EXPRESSION,

			EXPRESSION_A,
			EXPRESSION_A_MUL_X,
			EXPRESSION_A_MUL_X_PLUS_B,
			EXPRESSION_X_PLUS_A,
			EXPRESSION_X
		};

		enum Input : uint8_t
		{
			INVALID_INPUT,

			INPUT_HEALTH_CURRENT,
			INPUT_HEALTH_MAX,
			INPUT_MANA_CURRENT,
			INPUT_MANA_MAX,
			INPUT_LEVEL,
			INPUT_MANA_BASE,
			INPUT_SPELL_DAMAGE,
			INPUT_HEALING,
			INPUT_WEAPON,
			INPUT_WEAPON_AVERAGE,
			INPUT_RANGED,
			INPUT_RANGED_AVERAGE
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
			else if (t == "x_plus_a")
				return EXPRESSION_X_PLUS_A;
			else if (t == "x")
				return EXPRESSION_X;
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
				return INPUT_MANA_CURRENT;
			else if (t == "mana_max")
				return INPUT_MANA_MAX;
			else if (t == "mana_base")
				return INPUT_MANA_BASE;
			else if (t == "level")
				return INPUT_LEVEL;
			else if (t == "spell_damage")
				return INPUT_SPELL_DAMAGE;
			else if (t == "healing")
				return INPUT_HEALING;
			else if (t == "weapon")
				return INPUT_WEAPON;
			else if (t == "weapon_average")
				return INPUT_WEAPON_AVERAGE;
			else if (t == "ranged")
				return INPUT_RANGED;
			else if (t == "ranged_average")
				return INPUT_RANGED_AVERAGE;
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
			else if (aSource->m_type == SourceNode::TYPE_ARRAY)
			{
				m_expression = EXPRESSION_A;
				m_aLevelCurve = UIntCurve<uint32_t>(aSource);
			}
			else
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "expression")
					{
						m_expression = SourceToExpression(aChild);
					}
					else if(aChild->m_name == "x")
					{
						m_x = SourceToInput(aChild);
					}
					else if(aChild->m_name == "a")
					{
						if(aChild->m_type == SourceNode::TYPE_NUMBER)
							m_a = aChild->GetFloat();
						else
							m_aLevelCurve = UIntCurve<uint32_t>(aChild);
					}
					else if (aChild->m_name == "b")
					{
						if (aChild->m_type == SourceNode::TYPE_NUMBER)
							m_b = aChild->GetFloat();
						else
							m_bLevelCurve = UIntCurve<uint32_t>(aChild);
					}
					else if (aChild->m_name == "spread")
					{
						m_spread = aChild->GetFloat();
					}
					else if (aChild->m_name == "elite_multiplier")
					{
						m_eliteMultiplier = aChild->GetFloat();
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
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
			aWriter->WriteFloat(m_spread);
			m_aLevelCurve.ToStream(aWriter);
			m_bLevelCurve.ToStream(aWriter);

			aWriter->WriteFloat(m_eliteMultiplier);
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
			if (!aReader->ReadFloat(m_spread))
				return false;
			if(!m_aLevelCurve.FromStream(aReader))
				return false;
			if (!m_bLevelCurve.FromStream(aReader))
				return false;

			if(!aReader->IsEnd())
			{
				if (!aReader->ReadFloat(m_eliteMultiplier))
					return false;
			}

			return true;
		}

		bool 
		IsSet() const
		{
			return m_expression != INVALID_EXPRESSION;
		}

		float		Evaluate(
						RandomSource						aRandomSource,
						const Components::CombatPublic*		aCombatPublic,
						const Components::CombatPrivate*	aCombatPrivate) const;
		float		EvaluateEntityInstance(
						RandomSource						aRandomSource,
						const EntityInstance*				aEntityInstance) const;
		void		ToRange(
						const EntityInstance*				aEntityInstance,
						UIntRange&							aOut) const;
	
		// Public data
		Expression				m_expression = INVALID_EXPRESSION;
		Input					m_x = INVALID_INPUT;
		float					m_a = 0.0f;
		float					m_b = 0.0f;
		float					m_spread = 0.0f;
		float					m_eliteMultiplier = 1.0f;

		UIntCurve<uint32_t>		m_aLevelCurve;
		UIntCurve<uint32_t>		m_bLevelCurve;
	};
}