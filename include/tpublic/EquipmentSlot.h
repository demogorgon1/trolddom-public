#pragma once

namespace tpublic
{

	namespace EquipmentSlot
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_MAIN_HAND,
			ID_OFF_HAND,
			ID_RANGED,
			ID_HEAD,
			ID_CHEST,
			ID_LEGS,
			ID_FEET,
			ID_SHOULDERS,
			ID_WRISTS,
			ID_WAIST,
			ID_HANDS,
			ID_BACK,
			ID_NECK,
			ID_FINGER_1,
			ID_FINGER_2,
			ID_BAG_1,
			ID_BAG_2,
			ID_BAG_3,

			NUM_IDS
		};

		struct Info
		{
			const char* m_name;
			const char* m_tag;
			const char* m_displayName;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL, NULL },

			{ "main_hand",	NULL,			"Main-Hand" },
			{ "off_hand",	NULL,			"Off-Hand" },
			{ "ranged",		NULL,			"Ranged" },
			{ "head",		"head",			"Head" },
			{ "chest",		"chest",		"Chest" },
			{ "legs",		"legs",			"Legs" },
			{ "feet",		"feet",			"Feet" },
			{ "shoulders",	"shoulders",	"Shoulders" },
			{ "wrists",		"wrists",		"Wrists" },
			{ "waist",		"waist",		"Waist" },
			{ "hands",		"hands",		"Hands" },
			{ "back",		"back",			"Back" },
			{ "neck",		"neck",			"Neck" },
			{ "finger_1",	"finger",		"Finger" },
			{ "finger_2",	"finger",		"Finger" },
			{ "bag_1",		"bag",			"Bag" },
			{ "bag_2",		"bag",			"Bag" },
			{ "bag_3",		"bag",			"Bag" }
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
		Validate(
			Id				aId)
		{
			return (uint32_t)aId < (uint32_t)NUM_IDS && aId != INVALID_ID;
		}

	}

}