#pragma once

#include "ItemType.h"

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
			ID_TRINKET_1,
			ID_TRINKET_2,

			NUM_IDS
		};

		static const uint8_t NUM_IDS_V0 = ID_TRINKET_1;

		struct Info
		{
			const char*					m_name;
			const char*					m_tag;
			const char*					m_displayName;
			bool						m_alternative;
			std::vector<ItemType::Id>	m_itemTypes;
		};

		// IMPORTANT: Must match Id enum
		static const Info INFO[] =
		{
			{ NULL, NULL, NULL, false, {} },

			{ "main_hand",	NULL,			"Main-Hand",	false,	{} },
			{ "off_hand",	NULL,			"Off-Hand",		false,	{} },
			{ "ranged",		NULL,			"Ranged",		false,	{} },
			{ "head",		"head",			"Head",			false,	{ ItemType::ID_ARMOR_CLOTH, ItemType::ID_ARMOR_MAIL, ItemType::ID_ARMOR_PLATE } },
			{ "chest",		"chest",		"Chest",		false,	{ ItemType::ID_ARMOR_CLOTH, ItemType::ID_ARMOR_MAIL, ItemType::ID_ARMOR_PLATE } },
			{ "legs",		"legs",			"Legs",			false,	{ ItemType::ID_ARMOR_CLOTH, ItemType::ID_ARMOR_MAIL, ItemType::ID_ARMOR_PLATE } },
			{ "feet",		"feet",			"Feet",			false,	{ ItemType::ID_ARMOR_CLOTH, ItemType::ID_ARMOR_MAIL, ItemType::ID_ARMOR_PLATE } },
			{ "shoulders",	"shoulders",	"Shoulders",	false,	{ ItemType::ID_ARMOR_CLOTH, ItemType::ID_ARMOR_MAIL, ItemType::ID_ARMOR_PLATE } },
			{ "wrists",		"wrists",		"Wrists",		false,	{ ItemType::ID_ARMOR_CLOTH, ItemType::ID_ARMOR_MAIL, ItemType::ID_ARMOR_PLATE } },
			{ "waist",		"waist",		"Waist",		false,	{ ItemType::ID_ARMOR_CLOTH, ItemType::ID_ARMOR_MAIL, ItemType::ID_ARMOR_PLATE } },
			{ "hands",		"hands",		"Hands",		false,	{ ItemType::ID_ARMOR_CLOTH, ItemType::ID_ARMOR_MAIL, ItemType::ID_ARMOR_PLATE } },
			{ "back",		"back",			"Back",			false,	{ ItemType::ID_ARMOR_CLOTH } },
			{ "neck",		"neck",			"Neck",			false,	{} },
			{ "finger_1",	"finger",		"Finger",		false,	{} },
			{ "finger_2",	"finger",		"Finger",		true,	{} },
			{ "bag_1",		"bag",			"Bag",			false,	{} },
			{ "bag_2",		"bag",			"Bag",			true,	{} },
			{ "bag_3",		"bag",			"Bag",			true,	{} },
			{ "trinket_1",	"trinket",		"Trinket",		false,	{} },
			{ "trinket_2",	"trinket",		"Trinket",		true,	{} },
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
			std::string_view s(aString);
			for(uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				const Info* t = &INFO[i];
				if(s == t->m_name)
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