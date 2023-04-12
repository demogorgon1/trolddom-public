#pragma once

namespace kaos_public
{

	namespace Effect
	{

		enum Flag : uint32_t 
		{
			FLAG_CAN_MISS				= 0x00000001,
			FLAG_CAN_BE_BLOCKED			= 0x00000002,
			FLAG_CAN_BE_PARRIED			= 0x00000004,
			FLAG_CAN_BE_DODGED			= 0x00000008,
			FLAG_IS_MAGICAL				= 0x00000010
		};

		enum DamageType : uint8_t
		{
			DAMAGE_TYPE_PHYSICAL,
			DAMAGE_TYPE_COLD,		
			DAMAGE_TYPE_FIRE,			
			DAMAGE_TYPE_ARCANE,
			DAMAGE_TYPE_EVIL,		
			DAMAGE_TYPE_HOLY,			
			DAMAGE_TYPE_POISON
		};

		inline uint32_t	
		StringToFlag(
			const char* aString)
		{
			if (strcmp(aString, "can_miss") == 0)
				return FLAG_CAN_MISS;
			if (strcmp(aString, "can_be_blocked") == 0)
				return FLAG_CAN_BE_BLOCKED;
			if (strcmp(aString, "can_be_parried") == 0)
				return FLAG_CAN_BE_PARRIED;
			if (strcmp(aString, "can_be_dodged") == 0)
				return FLAG_CAN_BE_DODGED;
			if (strcmp(aString, "is_magical") == 0)
				return FLAG_IS_MAGICAL;
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
			ID_STUN,
			ID_KILL,

			NUM_IDS
		};

		// IMPORTANT: Must match Id enum
		static constexpr const char* EFFECT_NAMES[] =
		{
			NULL,
			"damage",
			"stun",
			"kill"
		};

		static_assert(sizeof(EFFECT_NAMES) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id
		StringToId(
			const char* aString)
		{
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				assert(EFFECT_NAMES[i] != NULL);
				if (strcmp(EFFECT_NAMES[i], aString) == 0)
					return (Id)i;
			}

			return INVALID_ID;
		}

	}

}