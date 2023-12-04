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
			"player_stats"
		};		

		static_assert(sizeof(COMPONENT_NAMES) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id
		StringToId(
			const char*		aString)
		{
			for(uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				assert(COMPONENT_NAMES[i] != NULL);
				if(strcmp(COMPONENT_NAMES[i], aString) == 0)
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