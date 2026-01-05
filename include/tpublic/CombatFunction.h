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
	class IWorldView;
	class Manifest;
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
			EXPRESSION_X,
			EXPRESSION_A_MUL_X_PLUS_B_MUL_Y,
			EXPRESSION_A_MUL_X_PLUS_B_MUL_Y_PLUS_C
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
			INPUT_RANGED_AVERAGE,
			INPUT_WEAPON_NORMALIZED,
			INPUT_WEAPON_AVERAGE_NORMALIZED,
			INPUT_ATTACK_POWER,
			INPUT_PVP_CONTROL_POINTS,
			INPUT_OWNER_SPELL_DAMAGE,
			INPUT_OWNER_HEALING,
			INPUT_MINION_WEAPON,
			INPUT_BLOCK_VALUE,
			INPUT_STRENGTH,
		};

		enum Entity : uint8_t
		{
			INVALID_ENTITY,

			ENTITY_SOURCE,
			ENTITY_TARGET
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
			else if (t == "a_mul_x_plus_b_mul_y")
				return EXPRESSION_A_MUL_X_PLUS_B_MUL_Y;
			else if (t == "x_plus_a")
				return EXPRESSION_X_PLUS_A;
			else if (t == "x")
				return EXPRESSION_X;
			else if (t == "a_mul_x_plus_b_mul_y_plus_c")
				return EXPRESSION_A_MUL_X_PLUS_B_MUL_Y_PLUS_C;
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
			else if (t == "weapon_normalized")
				return INPUT_WEAPON_NORMALIZED;
			else if (t == "weapon_average_normalized")
				return INPUT_WEAPON_AVERAGE_NORMALIZED;
			else if (t == "attack_power")
				return INPUT_ATTACK_POWER;
			else if (t == "block_value")
				return INPUT_BLOCK_VALUE;
			else if(t == "pvp_control_points")
				return INPUT_PVP_CONTROL_POINTS;
			else if(t == "owner_spell_damage")
				return INPUT_OWNER_SPELL_DAMAGE;
			else if (t == "owner_healing")
				return INPUT_OWNER_HEALING;
			else if (t == "minion_weapon")
				return INPUT_MINION_WEAPON;
			else if (t == "strength")
				return INPUT_STRENGTH;
			TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid input.", aSource->GetIdentifier());
			return INVALID_INPUT;
		}

		static Entity
		SourceToEntity(
			const SourceNode*					aSource)
		{
			std::string_view t(aSource->GetIdentifier());
			if (t == "source")
				return ENTITY_SOURCE;
			else if (t == "target")
				return ENTITY_TARGET;
			TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid entity.", aSource->GetIdentifier());
			return INVALID_ENTITY;
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
					else if (aChild->m_name == "y")
					{
						m_y = SourceToInput(aChild);
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
					else if (aChild->m_name == "c")
					{
						if (aChild->m_type == SourceNode::TYPE_NUMBER)
							m_c = aChild->GetFloat();
						else
							m_cLevelCurve = UIntCurve<uint32_t>(aChild);
					}
					else if (aChild->m_name == "spread")
					{
						m_spread = aChild->GetFloat();
					}
					else if (aChild->m_name == "elite_multiplier")
					{
						m_eliteMultiplier = aChild->GetFloat();
					}
					else if (aChild->m_name == "heroic_multiplier")
					{
						m_heroicMultiplier = aChild->GetFloat();
					}
					else if(aChild->m_name == "entity")
					{
						m_entity = SourceToEntity(aChild);
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
			aWriter->WritePOD<uint8_t>(0xFF); // This marks extended new format
			aWriter->WritePOD<uint8_t>(2); // Format tag

			aWriter->WritePOD(m_expression);
			aWriter->WritePOD(m_x);
			aWriter->WriteFloat(m_a);
			aWriter->WriteFloat(m_b);
			aWriter->WriteFloat(m_spread);
			m_aLevelCurve.ToStream(aWriter);
			m_bLevelCurve.ToStream(aWriter);
			aWriter->WriteFloat(m_eliteMultiplier);
			aWriter->WritePOD(m_y);
			aWriter->WritePOD(m_entity);
			aWriter->WriteFloat(m_c);
			m_cLevelCurve.ToStream(aWriter);
			aWriter->WriteFloat(m_heroicMultiplier);
		}

		bool
		FromStream(
			IReader*							aReader)
		{
			uint8_t version = 0;

			{
				uint8_t mark;
				if(aReader->Peek(&mark, 1) != 1)
					return false;
				if(mark == 0xFF)
				{
					if (!aReader->ReadPOD(mark)) // Skip
						return false;
					if(!aReader->ReadPOD(version))
						return false;
				}
			}

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
			if (!aReader->ReadFloat(m_eliteMultiplier))
				return false;
			if (!aReader->ReadPOD(m_y))
				return false;
			if(!aReader->ReadPOD(m_entity))
				return false;

			if(version >= 1)
			{
				if (!aReader->ReadFloat(m_c))
					return false;
				if (!m_cLevelCurve.FromStream(aReader))
					return false;
			}

			if(version >= 2)
			{
				if (!aReader->ReadFloat(m_heroicMultiplier))
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
						const Manifest*						aManifest,
						const IWorldView*					aWorldView,
						RandomSource						aRandomSource,
						float								aMultiplier,
						const EntityInstance*				aEntityInstance,
						const Components::CombatPublic*		aCombatPublic,
						const Components::CombatPrivate*	aCombatPrivate,
						bool								aIsOffHandAttack) const;
		float		EvaluateEntityInstance(
						const Manifest*						aManifest,
						const IWorldView*					aWorldView,
						RandomSource						aRandomSource,
						float								aMultiplier,
						const EntityInstance*				aEntityInstance,
						bool								aIsOffHandAttack) const;
		float		EvaluateSourceAndTargetEntityInstances(
						const Manifest*						aManifest,
						const IWorldView*					aWorldView,
						RandomSource						aRandomSource,
						float								aMultiplier,
						const EntityInstance*				aSourceEntityInstance,
						const EntityInstance*				aTargetEntityInstance,
						bool								aIsOffHandAttack) const;
		void		ToRange(
						const Manifest*						aManifest,
						const IWorldView*					aWorldView,
						float								aMultiplier,
						const EntityInstance*				aEntityInstance,
						bool								aIsOffHandAttack,
						UIntRange&							aOut) const;
	
		// Public data
		Expression				m_expression = INVALID_EXPRESSION;
		Input					m_x = INVALID_INPUT;
		Input					m_y = INVALID_INPUT;
		float					m_a = 0.0f;
		float					m_b = 0.0f;
		float					m_c = 0.0f;
		float					m_spread = 0.0f;
		float					m_eliteMultiplier = 1.0f;
		float					m_heroicMultiplier = 1.0f;
		Entity					m_entity = ENTITY_SOURCE;

		UIntCurve<uint32_t>		m_aLevelCurve;
		UIntCurve<uint32_t>		m_bLevelCurve;
		UIntCurve<uint32_t>		m_cLevelCurve;
	};
}