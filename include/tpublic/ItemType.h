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

			ID_CONSUMABLE,
			ID_TRADE_GOOD,
			ID_MISCELLANEOUS,

			NUM_IDS
		};

		enum Flag : uint16_t
		{
			FLAG_SWORD				= 0x0001,
			FLAG_MACE				= 0x0002,
			FLAG_AXE				= 0x0004,
			FLAG_KNIFE				= 0x0008,
			FLAG_TWO_HANDED			= 0x0010,
			FLAG_SHIELD				= 0x0020,
			FLAG_WEAPON				= 0x0040,
			FLAG_ARMOR				= 0x0080,

			FLAG_NO_RESTRICTIONS	= 0x4000,
			FLAG_NO_TOOLTIP			= 0x8000
		};

		struct Info
		{
			const char* m_name;
			const char* m_displayName;
			const char* m_displayNameCategory;
			uint16_t	m_flags;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL, NULL, NULL, 0 },

			{ "none",				"",						"",						0 },

			{ "armor_cloth",		"Cloth",				"Cloth Armor",			FLAG_ARMOR },
			{ "armor_mail",			"Mail",					"Mail Armor",			FLAG_ARMOR },
			{ "armor_plate",		"Plate",				"Plate Armor",			FLAG_ARMOR },

			{ "weapon_1h_sword",	"One-Handed Sword",		"One-Handed Swords",	FLAG_SWORD | FLAG_WEAPON },
			{ "weapon_1h_mace",		"One-Handed Mace",		"One-Handed Maces",		FLAG_MACE | FLAG_WEAPON },
			{ "weapon_1h_axe",		"One-Handed Axe",		"One-Handed Axes",		FLAG_AXE | FLAG_WEAPON },
			{ "weapon_1h_knife",	"Knife",				"Knives",				FLAG_WEAPON },

			{ "weapon_2h_sword",	"Two-Handed Sword",		"Two-Handed Swords",	FLAG_SWORD | FLAG_TWO_HANDED | FLAG_WEAPON },
			{ "weapon_2h_mace",		"Two-Handed Mace",		"Two-Handed Maces",		FLAG_MACE | FLAG_TWO_HANDED | FLAG_WEAPON },
			{ "weapon_2h_axe",		"Two-Handed Axe",		"Two-Handed Axes",		FLAG_AXE | FLAG_TWO_HANDED | FLAG_WEAPON },
			{ "weapon_2h_staff",	"Staff",				"Staves",				FLAG_TWO_HANDED | FLAG_WEAPON },
			
			{ "shield",				"Shield",				"Shields",				FLAG_SHIELD | FLAG_ARMOR },

			{ "consumable",			"Consumable",			"Consumables",			FLAG_NO_TOOLTIP | FLAG_NO_RESTRICTIONS },
			{ "trade_good",			"Trade Good",			"Trade Goods",			FLAG_NO_TOOLTIP | FLAG_NO_RESTRICTIONS },
			{ "miscellaneous",		"Miscellaneous",		"Miscellaneous",		FLAG_NO_TOOLTIP | FLAG_NO_RESTRICTIONS }
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