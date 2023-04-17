#pragma once

namespace kaos_public
{

	namespace Resource
	{

		enum Id : uint32_t
		{
			INVALID_ID,

			ID_MANA,
			ID_RAGE,
			ID_ENERGY,

			NUM_IDS
		};

		// IMPORTANT: Must match Id enum
		static constexpr const char* RESOURCE_NAMES[] =
		{
			NULL,
			"mana",
			"rage",
			"energy"
		};

		static_assert(sizeof(RESOURCE_NAMES) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id
		StringToId(
			const char* aString)
		{
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				assert(RESOURCE_NAMES[i] != NULL);
				if (strcmp(RESOURCE_NAMES[i], aString) == 0)
					return (Id)i;
			}

			return INVALID_ID;
		}

	}

}