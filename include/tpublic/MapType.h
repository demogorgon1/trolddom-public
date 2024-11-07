#pragma once

namespace tpublic
{

	namespace MapType
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_OPEN_WORLD,
			ID_PRIVATE,

			NUM_IDS
		};

		enum ResetMode : uint8_t
		{
			INVALID_RESET_MODE,

			RESET_MODE_MANUAL,
			RESET_MODE_BIDAILY,
			RESET_MODE_DAILY,
			RESET_MODE_WEEKLY,

			NUM_RESET_MODES
		};

		// IMPORTANT: must match Id enum
		static constexpr const char* STRINGS_ID[] =
		{
			NULL,
			"open_world",
			"private"
		};

		// IMPORTANT: must match ResetMode enum
		static constexpr const char* STRINGS_RESET_MODE[] =
		{
			NULL,
			"manual",
			"bidaily",
			"daily",
			"weekly"
		};

		static_assert(sizeof(STRINGS_ID) / sizeof(const char*) == (size_t)NUM_IDS);
		static_assert(sizeof(STRINGS_RESET_MODE) / sizeof(const char*) == (size_t)NUM_RESET_MODES);

		inline constexpr Id 
		StringToId(
			const char*		aString)
		{
			std::string_view s(aString);
			for(uint8_t i = 1; i < (uint8_t)NUM_IDS; i++)
			{
				if(s == STRINGS_ID[i])
					return (Id)i;
			}

			return INVALID_ID;
		}

		inline constexpr const char* 
		IdToString(
			Id				aId)
		{
			assert((uint8_t)aId < (uint8_t)NUM_IDS);
			const char* p = STRINGS_ID[aId];
			assert(p != NULL);
			return p;
		}

		inline constexpr ResetMode
		StringToResetMode(
			const char*		aString)
		{
			std::string_view s(aString);
			for(uint8_t i = 1; i < (uint8_t)NUM_RESET_MODES; i++)
			{
				if(s == STRINGS_RESET_MODE[i])
					return (ResetMode)i;
			}

			return INVALID_RESET_MODE;
		}

		inline constexpr const char* 
		ResetModeToString(
			ResetMode		aResetMode)
		{
			assert((uint8_t)aResetMode < (uint8_t)NUM_RESET_MODES);
			const char* p = STRINGS_RESET_MODE[aResetMode];
			assert(p != NULL);
			return p;
		}

	}

}