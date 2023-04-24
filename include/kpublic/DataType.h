#pragma once

namespace kpublic
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
			"aura"
		};

		static_assert(sizeof(STRINGS) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id 
		StringToId(
			const char*		aString)
		{
			for(uint8_t i = 1; i < (uint8_t)NUM_IDS; i++)
			{
				if(strcmp(STRINGS[i], aString) == 0)
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

	}

}