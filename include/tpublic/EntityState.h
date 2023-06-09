#pragma once

namespace tpublic
{

	namespace EntityState
	{
	
		enum Id : uint8_t
		{
			INVALID_ID,
			CONTINUE,
			DESTROY,

			ID_DEFAULT,
			ID_SPAWNING,
			ID_IN_COMBAT,
			ID_DEAD,
			ID_DESPAWNING,

			NUM_IDS
		};

		// IMPORTANT: Must match Id enum
		static constexpr const char* ENTITY_STATE_NAMES[] =
		{
			NULL,
			NULL,
			NULL,

			"default",
			"spawning",
			"in_combat",
			"dead",
			"despawning"
		};

		static_assert(sizeof(ENTITY_STATE_NAMES) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id
		StringToId(
			const char* aString)
		{
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				if(ENTITY_STATE_NAMES[i] != NULL)
				{
					if (strcmp(ENTITY_STATE_NAMES[i], aString) == 0)
						return (Id)i;
				}
			}

			return INVALID_ID;
		}

		inline constexpr bool
		CanBeAttacked(
			Id			aId)
		{
			switch(aId)
			{
			case ID_DEFAULT:	
			case ID_IN_COMBAT:
				return true;

			default:
				break;
			}
			return false;
		}

		inline constexpr bool
		CanMove(
			Id			aId)
		{
			switch(aId)
			{
			case ID_DEFAULT:	
			case ID_IN_COMBAT:
				return true;

			default:
				break;
			}
			return false;
		}

	}

}