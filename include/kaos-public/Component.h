#pragma once

namespace kaos_public
{

	namespace Component
	{
		
		enum Id : uint32_t
		{
			INVALID_ID,

			ID_COMBAT,
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

			NUM_IDS
		};

		// IMPORTANT: Must match Id enum
		static constexpr const char* COMPONENT_NAMES[] = 
		{
			NULL,
			"combat",
			"npc",
			"position",
			"player_public",
			"player_private",
			"inventory",
			"equipped_items",
			"talents",
			"wallet",
			"abilities",
			"display_name"
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

	}

}