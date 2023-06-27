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
			ID_WRISTS,
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
			const char* m_displayName;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL, NULL },

			{ "main_hand",	"Main-Hand" },
			{ "off_hand",	"Off-Hand" },
			{ "ranged",		"Ranged" },
			{ "head",		"Head" },
			{ "chest",		"Chest" },
			{ "legs",		"Legs" },
			{ "feet",		"Feet" },
			{ "wrists",		"Wrists" },
			{ "hands",		"Hands" },
			{ "back",		"Back" },
			{ "neck",		"Neck" },
			{ "finger_1",	"Finger 1" },
			{ "finger_2",	"Finger 2" },
			{ "bag_1",		"Bag 1" },
			{ "bag_2",		"Bag 2" },
			{ "bag_3",		"Bag 3" }
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