#pragma once

namespace tpublic
{

	namespace ObjectiveType
	{

		enum Id : uint32_t
		{
			INVALID_ID,

			ID_KILL_NPC,

			NUM_IDS
		};

		// IMPORTANT: Must match Id enum
		static constexpr const char* STRINGS[] =
		{
			NULL,
			"kill_npc"
		};

		static_assert(sizeof(STRINGS) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id
		StringToId(
			const char* aString)
		{
			std::string_view s(aString);
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				assert(STRINGS[i] != NULL);
				if (s == aString)
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