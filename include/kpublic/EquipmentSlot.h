#pragma once

namespace kpublic
{

	namespace EquipmentSlot
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_MAIN_HAND,
			ID_OFF_HAND,
			ID_HEAD,
			ID_CHEST,
			ID_LEGS,
			ID_FEET,
			ID_WRISTS,
			ID_HANDS,

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

			{ "main_hand",	"Main-Hand" },
			{ "off_hand",	"Off-Hand" },
			{ "head",		"Head" },
			{ "chest",		"Chest" },
			{ "legs",		"Legs" },
			{ "feet",		"Feet" },
			{ "wrists",		"Wrists" },
			{ "hands",		"Hands" }
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