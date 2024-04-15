#pragma once

namespace tpublic
{

	namespace DirectEffect
	{

		enum Flag : uint32_t 
		{
			FLAG_IS_MAGICAL				= 0x00000001,
			FLAG_GENERATE_RAGE			= 0x00000002,
			FLAG_CAN_BE_CRITICAL		= 0x00000004
		};

		enum DamageType : uint8_t
		{
			DAMAGE_TYPE_PHYSICAL,
			DAMAGE_TYPE_COLD,		
			DAMAGE_TYPE_FIRE,			
			DAMAGE_TYPE_ARCANE,
			DAMAGE_TYPE_EVIL,		
			DAMAGE_TYPE_HOLY,			
			DAMAGE_TYPE_POISON,

			NUM_DAMAGE_TYPES
		};

		static_assert(NUM_DAMAGE_TYPES <= 32); // Must fit into a bitfield

		enum DamageBase : uint8_t
		{
			DAMAGE_BASE_RANGE,
			DAMAGE_BASE_WEAPON
		};

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
			return 0;
		}

		inline DamageType
		StringToDamageType(
			const char*	aString)
		{
			if(strcmp(aString, "physical") == 0)
				return DAMAGE_TYPE_PHYSICAL;
			if (strcmp(aString, "cold") == 0)
				return DAMAGE_TYPE_COLD;
			if (strcmp(aString, "fire") == 0)
				return DAMAGE_TYPE_FIRE;
			if (strcmp(aString, "arcane") == 0)
				return DAMAGE_TYPE_ARCANE;
			if (strcmp(aString, "evil") == 0)
				return DAMAGE_TYPE_EVIL;
			if (strcmp(aString, "holy") == 0)
				return DAMAGE_TYPE_HOLY;
			if (strcmp(aString, "poison") == 0)
				return DAMAGE_TYPE_POISON;
			return DamageType(0);
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
			"move"
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