#pragma once

namespace tpublic
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
			uint8_t		m_color[3];
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL, NULL,				{ 255, 255, 255 } },

			{ "common",		"Common",	{ 255, 255, 255 } },
			{ "uncommon",	"Uncommon",	{ 10, 232, 90 } },
			{ "rare",		"Rare",		{ 67, 173, 225 } },
			{ "epic",		"Epic",		{ 215, 80, 219 } },
			{ "mythic",		"Mythic",	{ 255, 207, 107 } }
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

		inline constexpr bool
		ValidateId(
			Id				aId)
		{
			return (uint32_t)aId >= 1 && (uint32_t)aId < (uint32_t)NUM_IDS;
		}

	}

}