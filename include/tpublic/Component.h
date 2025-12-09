#pragma once

namespace tpublic
{

	namespace Component
	{
		
		enum Id : uint32_t
		{
			INVALID_ID,

			ID_COMBAT_PUBLIC,
			ID_COMBAT_PRIVATE,
			ID_NPC,
			ID_POSITION,
			ID_PLAYER_PUBLIC,
			ID_PLAYER_PRIVATE,
			ID_INVENTORY,
			ID_EQUIPPED_ITEMS,
			ID_TALENTS,
			ID_WALLET,
			ID_ABILITIES,
			ID_DISPLAY_NAME,
			ID_SPRITE,
			ID_PLAYER_COOLDOWNS,
			ID_ACTION_BARS,
			ID_THREAT_TARGET,
			ID_THREAT_SOURCE,
			ID_AURAS,
			ID_VISIBLE_AURAS,
			ID_LOOTABLE,
			ID_ENVIRONMENT,
			ID_OWNER,
			ID_TAG,
			ID_VENDOR_BUYBACK,
			ID_RESURRECTION_POINT,
			ID_GUILD_REGISTRAR,
			ID_TRADE,
			ID_ACTIVE_QUESTS,
			ID_COMPLETED_QUESTS,
			ID_PLAYER_STATS,
			ID_RECENT_ACHIEVEMENTS,
			ID_ACHIEVEMENTS,
			ID_GUILD_NAME,
			ID_ZONE_DISCOVERY,
			ID_OPENABLE,
			ID_REPUTATION,
			ID_DEITY_RELATIONS,
			ID_SHRINE,
			ID_DEITY_DISCOVERY,
			ID_AUCTIONEER,
			ID_INTERACTABLE_OBJECT,
			ID_PLAYER_AUCTIONS,
			ID_PLAYER_INBOX,
			ID_PLAYER_OUTBOX,
			ID_CHARACTER_INFO_VIEW,
			ID_GATEWAY,
			ID_PLAYER_WORLD_STASH,
			ID_SOUND_SOURCE,
			ID_STASH,
			ID_ABILITY_MODIFIERS,
			ID_VISIBILITY,
			ID_TRIGGER,
			ID_SPIRIT_STONE_DISCOVERY,
			ID_ACCOUNT_ACHIEVEMENTS,
			ID_VENDOR_STOCK,
			ID_PLAYER_MINIONS,
			ID_MINION_PUBLIC,
			ID_MINION_PRIVATE,
			ID_CONTROL_POINT,
			ID_KILL_CONTRIBUTION,
			ID_HOVER_TEXT,
			ID_RANDOM_ITEM_VENDOR,
			ID_CRITTER,
			ID_SURVIVAL_INFO,
			ID_PVP_RIFT,
			ID_RECENT_PVP_KILLS,
			ID_DIMINISHING_EFFECTS,

			NUM_IDS
		};

		// IMPORTANT: Must match Id enum
		static constexpr const char* COMPONENT_NAMES[] = 
		{
			NULL,
			"combat_public",
			"combat_private",
			"npc",
			"position",
			"player_public",
			"player_private",
			"inventory",
			"equipped_items",
			"talents",
			"wallet",
			"abilities",
			"display_name",
			"sprite",
			"player_cooldowns",
			"action_bars",
			"threat_target",
			"threat_source",
			"auras",
			"visible_auras",
			"lootable",
			"environment",
			"owner",
			"tag",
			"vendor_buyback",
			"resurrection_point",
			"guild_registrar",
			"trade",
			"active_quests",
			"completed_quests",
			"player_stats",
			"recent_achievements",
			"achievements",
			"guild_name",
			"zone_discovery",
			"openable",
			"reputation",
			"deity_relations",
			"shrine",
			"deity_discovery",
			"auctioneer",
			"interactable_object",
			"player_auctions",
			"player_inbox",
			"player_outbox",
			"character_info_view",
			"gateway",
			"player_world_stash",
			"sound_source",
			"stash",
			"ability_modifiers",
			"visibility",
			"trigger",
			"spirit_stone_discovery",
			"account_achievements",
			"vendor_stock",
			"player_minions",
			"minion_public",
			"minion_private",
			"control_point",
			"kill_contribution",
			"hover_text",
			"random_item_vendor",
			"critter",
			"survival_info",
			"pvp_rift",
			"recent_pvp_kills",
			"diminishing_effects"
		};

		static_assert(sizeof(COMPONENT_NAMES) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id
		StringToId(
			const char*		aString)
		{
			std::string_view t(aString);
			for(uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				assert(COMPONENT_NAMES[i] != NULL);
				if(t == COMPONENT_NAMES[i])
					return (Id)i;
			}

			return INVALID_ID;
		}

		inline constexpr const char*
		IdToString(
			uint32_t		aId)
		{
			if(aId < (uint32_t)NUM_IDS)
			{
				const char* p = COMPONENT_NAMES[aId];
				if(p != NULL)
					return p;
			}

			return "?";
		}

	}

}