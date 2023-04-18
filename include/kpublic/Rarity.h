#pragma once

namespace kpublic
{

	namespace Rarity
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_COMMON,
			ID_UNCOMMON,
			ID_RARE,
			ID_EPIC,
			ID_MYTHIC,

			NUM_IDS
		};

		struct Info
		{
			const char* m_name;
			const char* m_displayName;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL, NULL },

			{ "common",		"Common" },
			{ "uncommon",	"Uncommon" },
			{ "rare",		"Rare" },
			{ "epic",		"Epic" },
			{ "mythic",		"Mythic" }
		};

		static_assert(sizeof(INFO) / sizeof(Info) == NUM_IDS);

		inline constexpr const Info*
		GetInfo(
			Id				aId)
		{
			assert((uint32_t)aId < (uint32_t)NUM_IDS);
			return &INFO[aId];
		}

		inline constexpr Id
		StringToId(
			const char*		aString)
		{
			for(uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				const Info* t = &INFO[i];
				if(strcmp(t->m_name, aString) == 0)
					return (Id)i;
			}
			return INVALID_ID;
		}

	}

}