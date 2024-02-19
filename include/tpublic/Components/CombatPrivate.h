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

			enum Field
			{
				FIELD_WEAPON_DAMAGE_RANGE_MIN,
				FIELD_WEAPON_DAMAGE_RANGE_MAX,
				FIELD_WEAPON_COOLDOWN,
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
				FIELD_ARMOR
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_WEAPON_DAMAGE_RANGE_MIN, "weapon_damage_range_min", offsetof(CombatPrivate, m_weaponDamageRangeMin));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_WEAPON_DAMAGE_RANGE_MAX, "weapon_damage_range_max", offsetof(CombatPrivate, m_weaponDamageRangeMax));
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_WEAPON_COOLDOWN, "weapon_cooldown", offsetof(CombatPrivate, m_weaponCooldown));
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
			}

			void
			Reset()
			{
				m_weaponDamageRangeMin = 0;
				m_weaponDamageRangeMax = 0;
				m_weaponCooldown = 0;
				m_physicalCriticalStrikeChance = 0.0f;
				m_magicalCriticalStrikeChance = 0.0f;
				m_dodgeChance = 0.0f;
				m_parryChance = 0.0f;
				m_missChance = 0.0f;
				m_blockChance = 0.0f;
				m_manaRegenerationNotCastingPer5Sec = 0;
				m_manaRegenerationPer5Sec = 0;
				m_healthRegenNotInCombat = 0;
				m_healthRegenerationPer5Sec = 0;
				m_armor = 0;
			}

			// Public data
			uint32_t							m_weaponDamageRangeMin = 0;
			uint32_t							m_weaponDamageRangeMax = 0;
			int32_t								m_weaponCooldown = 0;
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
		};

	}

}