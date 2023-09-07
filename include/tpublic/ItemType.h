#pragma once

namespace tpublic
{

	namespace ItemType
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_NONE,

			ID_ARMOR_CLOTH,
			ID_ARMOR_MAIL,
			ID_ARMOR_PLATE,

			ID_WEAPON_1H_SWORD,
			ID_WEAPON_1H_MACE,
			ID_WEAPON_1H_AXE,
			ID_WEAPON_1H_KNIFE,

			ID_WEAPON_2H_SWORD,
			ID_WEAPON_2H_MACE,
			ID_WEAPON_2H_AXE,
			ID_WEAPON_2H_STAFF,

			ID_SHIELD,

			NUM_IDS
		};

		enum Flag : uint8_t
		{
			FLAG_SWORD		= 0x01,
			FLAG_MACE		= 0x02,
			FLAG_AXE		= 0x04,
			FLAG_KNIFE		= 0x08,
			FLAG_TWO_HANDED	= 0x10,
			FLAG_SHIELD		= 0x20
		};

		struct Info
		{
			const char* m_name;
			const char* m_displayName;			
			uint8_t		m_flags;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL, NULL, 0 },

			{ "none",				"",						0 },

			{ "armor_cloth",		"Cloth",				0 },
			{ "armor_mail",			"Mail",					0 },
			{ "armor_plate",		"Plate",				0 },

			{ "weapon_1h_sword",	"One-Handed Sword",		FLAG_SWORD },
			{ "weapon_1h_mace",		"One-Handed Mace",		FLAG_MACE },
			{ "weapon_1h_axe",		"One-Handed Axe",		FLAG_AXE },
			{ "weapon_1h_knife",	"Knife",				0 },

			{ "weapon_2h_sword",	"Two-Handed Sword",		FLAG_SWORD | FLAG_TWO_HANDED },
			{ "weapon_2h_mace",		"Two-Handed Mace",		FLAG_MACE | FLAG_TWO_HANDED },
			{ "weapon_2h_axe",		"Two-Handed Axe",		FLAG_AXE | FLAG_TWO_HANDED },
			{ "weapon_2h_staff",	"Staff",				FLAG_TWO_HANDED },
			
			{ "shield",				"Shield",				FLAG_SHIELD },
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