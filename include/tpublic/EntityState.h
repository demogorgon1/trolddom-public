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
			ID_EVADING,
			ID_DEAD,
			ID_DESPAWNING,
			ID_DESPAWNED,

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
			"evading",
			"dead",
			"despawning",
			"despawned"
		};

		static_assert(sizeof(ENTITY_STATE_NAMES) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id
		StringToId(
			const char* aString)
		{
			std::string_view s(aString);

			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				if(ENTITY_STATE_NAMES[i] != NULL)
				{
					if (s == ENTITY_STATE_NAMES[i])
						return (Id)i;
				}
			}

			return INVALID_ID;
		}

		inline constexpr const char*
		IdToString(
			Id			aId)
		{
			assert((uint8_t)aId < (uint8_t)NUM_IDS);
			const char* t = ENTITY_STATE_NAMES[aId];
			assert(t != NULL);
			return t;
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