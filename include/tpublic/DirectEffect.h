#pragma once

namespace tpublic
{

	namespace DirectEffect
	{

		enum Flag : uint32_t 
		{
			FLAG_IS_MAGICAL				= 0x00000001,
			FLAG_GENERATE_RAGE			= 0x00000002,
			FLAG_CAN_BE_CRITICAL		= 0x00000004,
			FLAG_LEECH					= 0x00000008,
			FLAG_SELF					= 0x00000010
		};

		enum DamageType : uint8_t
		{
			INVALID_DAMAGE_TYPE,

			DAMAGE_TYPE_PHYSICAL,
			DAMAGE_TYPE_BLEED,
			DAMAGE_TYPE_FROST,		
			DAMAGE_TYPE_FIRE,			
			DAMAGE_TYPE_ARCANE,
			DAMAGE_TYPE_UNHOLY,		
			DAMAGE_TYPE_HOLY,			
			DAMAGE_TYPE_POISON,
			DAMAGE_TYPE_NATURE,

			NUM_DAMAGE_TYPES
		};

		static_assert(NUM_DAMAGE_TYPES <= 32); // Must fit into a bitfield

		// IMPORTANT: Must match DamageType enum
		static constexpr const char* DAMAGE_TYPE_NAMES[] =
		{
			NULL,
			"Physical",
			"Bleed",
			"Frost",
			"Fire",
			"Arcane",
			"Unholy",
			"Holy",
			"Poison",
			"Nature"
		};

		static_assert(sizeof(DAMAGE_TYPE_NAMES) / sizeof(const char*) == (size_t)NUM_DAMAGE_TYPES);

		inline uint32_t	
		StringToFlag(
			const char* aString)
		{
			if (strcmp(aString, "is_magical") == 0)
				return FLAG_IS_MAGICAL;
			if (strcmp(aString, "generate_rage") == 0)
				return FLAG_GENERATE_RAGE;
			if (strcmp(aString, "can_be_critical") == 0)
				return FLAG_CAN_BE_CRITICAL;
			if (strcmp(aString, "leech") == 0)
				return FLAG_LEECH;
			if (strcmp(aString, "self") == 0)
				return FLAG_SELF;
			return 0;
		}

		inline DamageType
		StringToDamageType(
			const char*	aString)
		{
			if(strcmp(aString, "physical") == 0)
				return DAMAGE_TYPE_PHYSICAL;
			if (strcmp(aString, "bleed") == 0)
				return DAMAGE_TYPE_BLEED;
			if (strcmp(aString, "frost") == 0)
				return DAMAGE_TYPE_FROST;
			if (strcmp(aString, "fire") == 0)
				return DAMAGE_TYPE_FIRE;
			if (strcmp(aString, "arcane") == 0)
				return DAMAGE_TYPE_ARCANE;
			if (strcmp(aString, "unholy") == 0)
				return DAMAGE_TYPE_UNHOLY;
			if (strcmp(aString, "holy") == 0)
				return DAMAGE_TYPE_HOLY;
			if (strcmp(aString, "poison") == 0)
				return DAMAGE_TYPE_POISON;
			if (strcmp(aString, "nature") == 0)
				return DAMAGE_TYPE_NATURE;
			return DamageType(0);
		}

		inline const char*
		DamageTypeToName(
			DamageType	aDamageType)
		{
			assert((uint32_t)aDamageType >= 0 && (uint32_t)aDamageType < (uint32_t)NUM_DAMAGE_TYPES);
			return DAMAGE_TYPE_NAMES[aDamageType];
		}

		inline bool
		CheckDamageTypeMask(
			DamageType	aDamageType,
			uint32_t	aTypeMask)
		{
			return ((1 << (uint32_t)aDamageType) & aTypeMask) != 0;
		}

		enum Id : uint32_t
		{
			INVALID_ID,

			ID_DAMAGE,
			ID_HEAL,
			ID_APPLY_AURA,
			ID_KILL,
			ID_PUSH,
			ID_THREAT,
			ID_SIMPLE,
			ID_FISHING,
			ID_CONSUME_AND_PRODUCE,
			ID_MOVE,
			ID_MODIFY_RESOURCE,
			ID_INTERRUPT,
			ID_SPAWN_ENTITY,
			ID_REMOVE_AURA,
			ID_LEARN_PROFESSION_ABILITY,
			ID_MODIFY_FACTION,
			ID_SPAWN_MINION,
			ID_TRIGGER_ABILITY,
			ID_START_ROUTE,

			NUM_IDS
		};

		// IMPORTANT: Must match Id enum
		static constexpr const char* EFFECT_NAMES[] =
		{
			NULL,
			"damage",
			"heal",
			"apply_aura",
			"kill",
			"push",
			"threat",
			"simple",
			"fishing",
			"consume_and_produce",
			"move",
			"modify_resource",
			"interrupt",
			"spawn_entity",
			"remove_aura",
			"learn_profession_ability",
			"modify_faction",
			"spawn_minion",
			"trigger_ability",
			"start_route"
		};

		static_assert(sizeof(EFFECT_NAMES) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id
		StringToId(
			const char* aString)
		{
			std::string_view s(aString);
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				assert(EFFECT_NAMES[i] != NULL);
				if (s == EFFECT_NAMES[i])
					return (Id)i;
			}

			return INVALID_ID;
		}

	}

}