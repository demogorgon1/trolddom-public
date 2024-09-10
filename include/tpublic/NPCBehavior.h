#pragma once

namespace tpublic
{

	namespace NPCBehavior
	{
	
		enum Id : uint8_t
		{
			INVALID_ID,

			ID_DO_NOTHING,
			ID_WANDERING,
			ID_USE_ABILITY,
			ID_PATROLLING,

			NUM_IDS
		};

		// IMPORTANT: Must match Id enum
		static constexpr const char* STRINGS[] =
		{
			NULL,

			"do_nothing",
			"wandering",
			"use_ability",
			"patrolling"
		};

		static_assert(sizeof(STRINGS) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id
		StringToId(
			const char* aString)
		{
			std::string_view s(aString);
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				if(STRINGS[i] != NULL)
				{
					if (s == STRINGS[i])
						return (Id)i;
				}
			}

			return INVALID_ID;
		}

	}

}