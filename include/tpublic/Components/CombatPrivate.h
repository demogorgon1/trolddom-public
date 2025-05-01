#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct CombatPrivate
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_COMBAT_PRIVATE;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			enum PrivateFlag : uint8_t
			{
				PRIVATE_FLAG_IMMUNE_TO_STUN			= 0x01,
				PRIVATE_FLAG_IMMUNE_TO_IMMOBILIZE	= 0x02,
				PRIVATE_FLAG_IMMUNE_TO_TAUNT		= 0x04,
				PRIVATE_FLAG_IMMUNE_TO_SLOW			= 0x08
			};

			enum Field
			{
				FIELD_WEAPON_DAMAGE_RANGE_MIN,
				FIELD_WEAPON_DAMAGE_RANGE_MAX,
				FIELD_WEAPON_COOLDOWN,
				FIELD_RANGED_DAMAGE_RANGE_MIN,
				FIELD_RANGED_DAMAGE_RANGE_MAX,
				FIELD_RANGED_COOLDOWN,
				FIELD_PHYSICAL_CRITICAL_STRIKE_CHANCE,
				FIELD_MAGICAL_CRITICAL_STRIKE_CHANCE,
				FIELD_DODGE_CHANCE,
				FIELD_PARRY_CHANCE,
				FIELD_MISS_CHANCE,
				FIELD_BLOCK_CHANCE,
				FIELD_MANA_REGEN_NOT_CASTING,
				FIELD_MANA_REGEN,
				FIELD_HEALTH_REGEN_NOT_IN_COMBAT,
				FIELD_HEALTH_REGEN,
				FIELD_ARMOR,
				FIELD_BLOCK_VALUE,
				FIELD_RAGE_DECAY_NOT_IN_COMBAT,
				FIELD_RAGE_GENERATION_IN_COMBAT,
				FIELD_BASE_MANA,
				FIELD_SPELL_DAMAGE,
				FIELD_HEALING,
				FIELD_PRIVATE_FLAGS,
				FIELD_ATTACK_POWER,
				FIELD_RESOURCE_COST_MULTIPLIER
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_WEAPON_DAMAGE_RANGE_MIN, "weapon_damage_range_min", offsetof(CombatPrivate, m_weaponDamageRangeMin));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_WEAPON_DAMAGE_RANGE_MAX, "weapon_damage_range_max", offsetof(CombatPrivate, m_weaponDamageRangeMax));
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_WEAPON_COOLDOWN, "weapon_cooldown", offsetof(CombatPrivate, m_weaponCooldown));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_WEAPON_DAMAGE_RANGE_MIN, "ranged_damage_range_min", offsetof(CombatPrivate, m_rangedDamageRangeMin));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_WEAPON_DAMAGE_RANGE_MAX, "ranged_damage_range_max", offsetof(CombatPrivate, m_rangedDamageRangeMax));
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_WEAPON_COOLDOWN, "ranged_cooldown", offsetof(CombatPrivate, m_rangedCooldown));
				aSchema->Define(ComponentSchema::TYPE_FLOAT, FIELD_PHYSICAL_CRITICAL_STRIKE_CHANCE, "physical_critical_strike_chance", offsetof(CombatPrivate, m_physicalCriticalStrikeChance));
				aSchema->Define(ComponentSchema::TYPE_FLOAT, FIELD_MAGICAL_CRITICAL_STRIKE_CHANCE, "magical_critical_strike_chance", offsetof(CombatPrivate, m_magicalCriticalStrikeChance));
				aSchema->Define(ComponentSchema::TYPE_FLOAT, FIELD_DODGE_CHANCE, "dodge_chance", offsetof(CombatPrivate, m_dodgeChance));
				aSchema->Define(ComponentSchema::TYPE_FLOAT, FIELD_PARRY_CHANCE, "parry_chance", offsetof(CombatPrivate, m_parryChance));
				aSchema->Define(ComponentSchema::TYPE_FLOAT, FIELD_MISS_CHANCE, "miss_chance", offsetof(CombatPrivate, m_missChance));
				aSchema->Define(ComponentSchema::TYPE_FLOAT, FIELD_BLOCK_CHANCE, "block_chance", offsetof(CombatPrivate, m_blockChance));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_MANA_REGEN_NOT_CASTING, "mana_regen_not_casting", offsetof(CombatPrivate, m_manaRegenerationNotCastingPer5Sec));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_MANA_REGEN, "mana_regen", offsetof(CombatPrivate, m_manaRegenerationPer5Sec));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_HEALTH_REGEN_NOT_IN_COMBAT, "health_regen_not_in_combat", offsetof(CombatPrivate, m_healthRegenNotInCombat));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_HEALTH_REGEN, "health_regen", offsetof(CombatPrivate, m_healthRegenerationPer5Sec));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_ARMOR, "armor", offsetof(CombatPrivate, m_armor));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_BLOCK_VALUE, "block_value", offsetof(CombatPrivate, m_blockValue));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_RAGE_DECAY_NOT_IN_COMBAT, "rage_decay_not_in_combat", offsetof(CombatPrivate, m_rageDecayNotInCombatPer5Sec));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_BASE_MANA, "base_mana", offsetof(CombatPrivate, m_baseMana));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_RAGE_GENERATION_IN_COMBAT, "rage_generation_in_combat", offsetof(CombatPrivate, m_rageGenerationInCombatPer5Sec));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_SPELL_DAMAGE, "spell_damage", offsetof(CombatPrivate, m_spellDamage));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_HEALING, "healing", offsetof(CombatPrivate, m_healing));
				aSchema->DefineCustomPODNoSource<uint8_t>(FIELD_PRIVATE_FLAGS, offsetof(CombatPrivate, m_privateFlags));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_ATTACK_POWER, "attack_power", offsetof(CombatPrivate, m_attackPower));
				aSchema->Define(ComponentSchema::TYPE_FLOAT, FIELD_RESOURCE_COST_MULTIPLIER, "resource_cost_multiplier", offsetof(CombatPrivate, m_resourceCostMultiplier));

				aSchema->AddSourceModifier<CombatPrivate>("immune_to_stun", [](
					CombatPrivate*		aCombatPrivate,
					const SourceNode*	/*aSource*/)
				{
					aCombatPrivate->m_privateFlags |= PRIVATE_FLAG_IMMUNE_TO_STUN;
				});

				aSchema->AddSourceModifier<CombatPrivate>("immune_to_immobilize", [](
					CombatPrivate*		aCombatPrivate,
					const SourceNode*	/*aSource*/)
				{
					aCombatPrivate->m_privateFlags |= PRIVATE_FLAG_IMMUNE_TO_IMMOBILIZE;
				});

				aSchema->AddSourceModifier<CombatPrivate>("immune_to_taunt", [](
					CombatPrivate*		aCombatPrivate,
					const SourceNode*	/*aSource*/)
				{
					aCombatPrivate->m_privateFlags |= PRIVATE_FLAG_IMMUNE_TO_TAUNT;
				});

				aSchema->AddSourceModifier<CombatPrivate>("immune_to_slow", [](
					CombatPrivate*		aCombatPrivate,
					const SourceNode*	/*aSource*/)
				{
					aCombatPrivate->m_privateFlags |= PRIVATE_FLAG_IMMUNE_TO_SLOW;
				});
			}

			void
			Reset()
			{
				m_weaponDamageRangeMin = 0;
				m_weaponDamageRangeMax = 0;
				m_weaponCooldown = 0;
				m_rangedDamageRangeMin = 0;
				m_rangedDamageRangeMax = 0;
				m_rangedCooldown = 0;
				m_physicalCriticalStrikeChance = 5.0f;
				m_magicalCriticalStrikeChance = 5.0f;
				m_dodgeChance = 5.0f;
				m_parryChance = 5.0f;
				m_missChance = 5.0f;
				m_blockChance = 5.0f;
				m_manaRegenerationNotCastingPer5Sec = 0;
				m_manaRegenerationPer5Sec = 0;
				m_healthRegenNotInCombat = 0;
				m_healthRegenerationPer5Sec = 0;
				m_armor = 0;
				m_blockValue = 0;
				m_rageDecayNotInCombatPer5Sec = 6;
				m_rageGenerationInCombatPer5Sec = 0;
				m_baseMana = 0;
				m_spellDamage = 0;
				m_healing = 0;
				m_privateFlags = 0;
				m_attackPower = 0;
				m_resourceCostMultiplier = 1.0f;
			}

			bool
			IsImmuneToSomething() const
			{
				uint8_t allImmuneFlags = PRIVATE_FLAG_IMMUNE_TO_STUN | PRIVATE_FLAG_IMMUNE_TO_IMMOBILIZE | PRIVATE_FLAG_IMMUNE_TO_SLOW | PRIVATE_FLAG_IMMUNE_TO_TAUNT;
				return (m_privateFlags & allImmuneFlags) != 0;
			}

			// Public data
			uint32_t							m_weaponDamageRangeMin = 0;
			uint32_t							m_weaponDamageRangeMax = 0;
			int32_t								m_weaponCooldown = 0;
			uint32_t							m_rangedDamageRangeMin = 0;
			uint32_t							m_rangedDamageRangeMax = 0;
			int32_t								m_rangedCooldown = 0;
			float								m_physicalCriticalStrikeChance = 0.0f;
			float								m_magicalCriticalStrikeChance = 0.0f;
			float								m_dodgeChance = 0.0f;
			float								m_parryChance = 0.0f;
			float								m_missChance = 0.0f;
			float								m_blockChance = 0.0f;
			uint32_t							m_manaRegenerationNotCastingPer5Sec = 0;
			uint32_t							m_manaRegenerationPer5Sec = 0;
			uint32_t							m_healthRegenNotInCombat = 0;
			uint32_t							m_healthRegenerationPer5Sec = 0;
			uint32_t							m_armor = 0;
			uint32_t							m_blockValue = 0;
			uint32_t							m_rageDecayNotInCombatPer5Sec = 6;
			uint32_t							m_rageGenerationInCombatPer5Sec = 0;
			uint32_t							m_baseMana = 0;
			uint32_t							m_spellDamage = 0;
			uint32_t							m_healing = 0;
			uint32_t							m_attackPower = 0;
			uint8_t								m_privateFlags = 0;
			float								m_resourceCostMultiplier = 1.0f;
		};

	}

}