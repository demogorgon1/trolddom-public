#pragma once

namespace tpublic
{

	namespace System
	{

		enum Id : uint32_t
		{
			INVALID_ID,

			ID_COMBAT,
			ID_NPC,
			ID_ENVIRONMENT,
			ID_HEALTH_REGEN,
			ID_MANA_REGEN,
			ID_OPENABLE,

			NUM_IDS
		};

		// IMPORTANT: Must match Id enum
		static constexpr const char* SYSTEM_NAMES[] =
		{
			NULL,
			"combat",
			"npc",
			"environment",
			"health_regen",
			"mana_regen",
			"openable"
		};

		static_assert(sizeof(SYSTEM_NAMES) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id
		StringToId(
			const char* aString)
		{
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				assert(SYSTEM_NAMES[i] != NULL);
				if (strcmp(SYSTEM_NAMES[i], aString) == 0)
					return (Id)i;
			}

			return INVALID_ID;
		}

		inline constexpr const char*
		IdToString(
			uint32_t	aId)
		{
			if((uint32_t)aId < (uint32_t)NUM_IDS)
			{
				const char* p = SYSTEM_NAMES[aId];
				if(p != NULL)
					return p;
			}

			return "?";
		}

	}

}