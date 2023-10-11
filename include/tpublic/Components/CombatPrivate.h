#pragma once

#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct CombatPrivate
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_COMBAT_PRIVATE;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

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
				FIELD_BLOCK_CHANCE
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_WEAPON_DAMAGE_RANGE_MIN, "weapon_damage_range_min", offsetof(CombatPrivate, m_weaponDamageRangeMin));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_WEAPON_DAMAGE_RANGE_MAX, "weapon_damage_range_max", offsetof(CombatPrivate, m_weaponDamageRangeMax));
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_WEAPON_COOLDOWN, "weapon_cooldown", offsetof(CombatPrivate, m_weaponCooldown));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_PHYSICAL_CRITICAL_STRIKE_CHANCE, "physical_critical_strike_chance", offsetof(CombatPrivate, m_physicalCriticalStrikeChance));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_MAGICAL_CRITICAL_STRIKE_CHANCE, "magical_critical_strike_chance", offsetof(CombatPrivate, m_magicalCriticalStrikeChance));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_DODGE_CHANCE, "dodge_chance", offsetof(CombatPrivate, m_dodgeChance));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_PARRY_CHANCE, "parry_chance", offsetof(CombatPrivate, m_parryChance));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_MISS_CHANCE, "miss_chance", offsetof(CombatPrivate, m_missChance));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_BLOCK_CHANCE, "block_chance", offsetof(CombatPrivate, m_blockChance));
			}

			void
			Reset()
			{
				m_weaponDamageRangeMin = 0;
				m_weaponDamageRangeMax = 0;
				m_weaponCooldown = 0;
				m_physicalCriticalStrikeChance = (5 * uint64_t(UINT32_MAX) / 100);
				m_magicalCriticalStrikeChance = (5 * uint64_t(UINT32_MAX) / 100);
				m_dodgeChance = (5 * uint64_t(UINT32_MAX) / 100);
				m_parryChance = (5 * uint64_t(UINT32_MAX) / 100);
				m_missChance = (5 * uint64_t(UINT32_MAX) / 100);
				m_blockChance = (5 * uint64_t(UINT32_MAX) / 100);
			}

			// Public data
			uint32_t							m_weaponDamageRangeMin = 0;
			uint32_t							m_weaponDamageRangeMax = 0;
			int32_t								m_weaponCooldown = 0;
			uint32_t							m_physicalCriticalStrikeChance = 0;
			uint32_t							m_magicalCriticalStrikeChance = 0;
			uint32_t							m_dodgeChance = 0;
			uint32_t							m_parryChance = 0;
			uint32_t							m_missChance = 0;
			uint32_t							m_blockChance = 0;
		};

	}

}