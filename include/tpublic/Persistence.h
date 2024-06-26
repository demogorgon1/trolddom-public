#pragma once

namespace tpublic::Persistence
{

	enum Id : uint8_t
	{
		ID_NONE,

		// Player 
		ID_ACCOUNT,
		ID_VOLATILE,
		ID_MAIN,
		ID_STATS,
		ID_PLAYER_WORLD_STASH,

		NUM_IDS
	};

	// Elsewhere we make the assumption that persistence id will fit in 3 bits
	static_assert((uint8_t)NUM_IDS <= 8);

	enum PlayerPersistenceType
	{
		PLAYER_PERSISTENCE_TYPE_NONE,
		PLAYER_PERSISTENCE_TYPE_PER_ACCOUNT,
		PLAYER_PERSISTENCE_TYPE_PER_CHARACTER
	};

	struct Info
	{
		const char*				m_name;
		PlayerPersistenceType	m_playerPersistenceType;
	};

	// IMPORTANT: Must match Id enum
	static constexpr const Info INFO[] =
	{
		{ NULL, PLAYER_PERSISTENCE_TYPE_NONE },

		// Player
		{ "account",			PLAYER_PERSISTENCE_TYPE_PER_ACCOUNT },
		{ "volatile",			PLAYER_PERSISTENCE_TYPE_PER_CHARACTER },
		{ "main",				PLAYER_PERSISTENCE_TYPE_PER_CHARACTER },
		{ "stats",				PLAYER_PERSISTENCE_TYPE_PER_CHARACTER },
		{ "player_world_stash",	PLAYER_PERSISTENCE_TYPE_PER_CHARACTER }
	};

	static_assert(sizeof(INFO) / sizeof(Info) == NUM_IDS);

	inline constexpr const Info*
	GetInfo(
		Id					aId)
	{
		assert((uint32_t)aId < (uint32_t)NUM_IDS);
		return &INFO[aId];
	}

	inline constexpr Id
	StringToId(
		const char*			aString)
	{
		std::string_view s(aString);
		for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
		{
			const Info* t = &INFO[i];
			if (t->m_name != NULL && s == t->m_name)
				return (Id)i;
		}
		return ID_NONE;
	}

}