#pragma once

namespace tpublic
{

	namespace DataType
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_ABILITY,
			ID_CLASS,
			ID_ENTITY,
			ID_MAP,
			ID_MAP_ENTITY_SPAWN,
			ID_MAP_PLAYER_SPAWN,
			ID_MAP_PALETTE,
			ID_TALENT,
			ID_TALENT_TREE,
			ID_SPRITE,
			ID_FACTION,
			ID_ITEM,
			ID_AURA,
			ID_MAP_PORTAL,
			ID_LOOT_TABLE,
			ID_LOOT_GROUP,
			ID_MAP_TRIGGER,
			ID_PARTICLE_SYSTEM,
			ID_DIALOGUE_SCREEN,
			ID_EXPRESSION,
			ID_DIALOGUE_ROOT,
			ID_NPC_BEHAVIOR_STATE,
			ID_MAP_SEGMENT,
			ID_MAP_SEGMENT_CONNECTOR,
			ID_ZONE,
			ID_CREATURE_TYPE,
			ID_OBJECTIVE,
			ID_QUEST,
			ID_ACHIEVEMENT,
			ID_ACHIEVEMENT_CATEGORY,
			ID_PROFESSION,
			ID_TAG,
			ID_WORD_GENERATOR,
			ID_TAG_CONTEXT,
			ID_PANTHEON,
			ID_DEITY,
			ID_COOLDOWN,
			ID_SOUND,
			ID_TERRAIN,
			ID_NOISE,
			ID_NAME_TEMPLATE,
			ID_DOODAD,
			ID_PLAYER_WORLD_TYPE,
			ID_WALL,
			ID_ABILITY_MODIFIER,
			ID_ENCOUNTER,
			ID_EMOTE,
			ID_ROUTE,
			ID_TILE_MODIFIER,
			ID_CLIFF_STYLE,
			ID_MAP_CLIFF,
			ID_CONTEXT_HELP,

			NUM_IDS
		};

		// IMPORTANT: must match Id enum
		static constexpr const char* STRINGS[] =
		{
			NULL,
			"ability",
			"class",
			"entity",
			"map",
			"map_entity_spawn",
			"map_player_spawn",
			"map_palette",
			"talent",
			"talent_tree",
			"sprite",
			"faction",
			"item",
			"aura",
			"map_portal",
			"loot_table",
			"loot_group",
			"map_trigger",
			"particle_system",
			"dialogue_screen",
			"expression",
			"dialogue_root",
			"npc_behavior_state",
			"map_segment",
			"map_segment_connector",
			"zone",
			"creature_type",
			"objective",
			"quest",
			"achievement",
			"achievement_category",
			"profession",
			"tag",
			"word_generator",
			"tag_context",
			"pantheon",
			"deity",
			"cooldown",
			"sound",
			"terrain",
			"noise",
			"name_template",
			"doodad",
			"player_world_type",
			"wall",
			"ability_modifier",
			"encounter",
			"emote",
			"route",
			"tile_modifier",
			"cliff_style",
			"map_cliff",
			"context_help"
		};

		static_assert(sizeof(STRINGS) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id 
		StringToId(
			const char*		aString)
		{
			std::string_view t(aString);
			for(uint8_t i = 1; i < (uint8_t)NUM_IDS; i++)
			{
				if(t == STRINGS[i])
					return (Id)i;
			}

			return INVALID_ID;
		}

		inline constexpr const char* 
		IdToString(
			Id				aId)
		{
			assert((uint8_t)aId < (uint8_t)NUM_IDS);
			const char* p = STRINGS[aId];
			assert(p != NULL);
			return p;
		}

		inline constexpr bool
		ValidateId(
			Id				aId)
		{
			return (uint8_t)aId > 0 && (uint8_t)aId < (uint8_t)NUM_IDS;
		}

	}

}