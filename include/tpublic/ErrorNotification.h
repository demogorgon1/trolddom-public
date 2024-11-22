#pragma once

namespace tpublic
{

	namespace ErrorNotification
	{
		
		enum Id : uint8_t
		{
			INVALID_ID,

			ID_CANNOT_DO_THAT_NOW,
			ID_TARGET_TOO_FAR_AWAY,
			ID_NOT_ENOUGH_RESOURCES,
			ID_NO_TARGET,
			ID_CANNOT_TARGET_SELF,
			ID_TARGET_IS_DEAD,
			ID_INVALID_TARGET,
			ID_NO_LINE_OF_SIGHT,
			ID_MISSING_INGREDIENTS,
			ID_NOT_ENOUGH_INVENTORY_SPACE,
			ID_STUNNED,
			ID_ALREADY_ON_QUEST,
			ID_INVALID_REQUEST,
			ID_NOT_AVAILABLE,
			ID_SELECT_REWARD,
			ID_CANNOT_AFFORD_DEPOSIT,
			ID_CANNOT_DELETE_MAIL_WITH_ATTACHMENTS,
			ID_OUTBOX_IS_FULL,
			ID_CANNOT_USE_ITEM,
			ID_YOU_DO_NOT_HAVE_YOUR_OWN_WORLD,
			ID_CANNOT_SELL_ITEM,
			ID_COULD_NOT_DO_THAT,
			ID_FAILED_TO_CREATE_WORLD,
			ID_IMMOBILIZED,
			ID_ITEM_NOT_EQUIPPED,
			ID_MISSING_ITEM,
			ID_TARGET_TOO_CLOSE,
			ID_RANGED_WEAPON_REQUIRED,
			ID_NO_OTHER_SPIRIT_STONES_DISCOVERED,
			ID_CANNOT_FIND_PATH,
			ID_OUT_OF_STOCK,
			ID_DISMISS_CURRENT_MINION,
			ID_TOO_MANY_MINIONS,
			ID_AMOUNT_TOO_LARGE,
			ID_TOO_LOW_LEVEL,
			ID_CANNOT_DO_THAT_HERE,
			ID_MUST_BE_IN_A_PARTY_TO_ENTER,
			ID_CANNOT_LOOT_THAT,

			NUM_IDS
		};

		inline constexpr bool
		ValidateId(
			Id					aId)
		{
			return (uint32_t)aId > 0 && (uint32_t)aId < (uint32_t)NUM_IDS;
		}
		
		struct Info
		{
			const char*							m_string;
		};

		// IMPORTANT: Must match Id enum
		static const Info INFO[] =
		{
			{ NULL },
			{ "Can't do that now!" },
			{ "Target too far away!" },
			{ "Not enough resources!" },
			{ "No target!" },
			{ "Can't target self!" },
			{ "Target is dead!" },
			{ "Invalid target!" },
			{ "No line of sight!" },
			{ "Missing ingredients!" },
			{ "Not enough inventory space!" },
			{ "Stunned!" },
			{ "Already on quest!" },
			{ "Invalid request!" },
			{ "Not available!" },
			{ "Select reward!" },
			{ "Can't afford deposit!" },
			{ "Can't delete mail with attachments!"	},
			{ "Outbox is full!"	},
			{ "Can't use item!" },
			{ "You don't have your own world!" },
			{ "Can't sell item!" },
			{ "Could not do that!" },
			{ "Failed to create world!" },
			{ "Immobilized!" },
			{ "Item not equipped!" },
			{ "Missing item!" },
			{ "Target too close!" },
			{ "Ranged weapon required!" },
			{ "No other spirit stones discovered!" },
			{ "Can't find path!" },
			{ "Out of stock!" },
			{ "Dismiss current minion!" },
			{ "Too many minions!" },
			{ "Amount too large! "},
			{ "Too low level! "},
			{ "Cannot do that here!" },
			{ "Must be in a party to enter!" },
			{ "Can't loot that!" }
		};

		static_assert(sizeof(INFO) / sizeof(Info) == NUM_IDS);

		inline constexpr const Info*
		GetInfo(
			Id					aId)
		{
			assert(aId != INVALID_ID);
			assert((uint32_t)aId < (uint32_t)NUM_IDS);
			return &INFO[aId];
		}
	}

}