#pragma once

namespace tpublic
{

	namespace MapGenerator
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_DUNGEON,

			NUM_IDS
		};

		// IMPORTANT: Must match Id enum
		static constexpr const char* STRINGS[] =
		{
			NULL,
			"dungeon"
		};

		static_assert(sizeof(STRINGS) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id
		StringToId(
			const char* aString)
		{
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				assert(STRINGS[i] != NULL);
				if (strcmp(STRINGS[i], aString) == 0)
					return (Id)i;
			}

			return INVALID_ID;
		}

		inline constexpr bool
		ValidateId(
			Id			aId)
		{
			return (uint32_t)aId >= 1 && (uint32_t)aId < (uint32_t)NUM_IDS;
		}

	}

}