#pragma once

namespace tpublic
{

	namespace MinionCommand
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_MOVE,
			ID_ATTACK,
			ID_HEAL,

			NUM_IDS
		};

		struct Info
		{
			const char* m_name;
			const char* m_displayName;
			const char* m_iconSpriteName;
			bool		m_targetPosition;
			uint32_t	m_priority; // 0 = highest prio
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL, NULL, NULL },

			{ "move",		"Move",		"icon_move_command",	true,	1 },
			{ "attack",		"Attack",	"icon_attack_command",	false,	2 },
			{ "heal",		"Heal",		"icon_heal_command",	false,	0 },
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
			const char* aString)
		{
			std::string_view s(aString);
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				const Info* t = &INFO[i];
				if (s == t->m_name)
					return (Id)i;
			}
			return INVALID_ID;
		}

		inline constexpr bool
		ValidateId(
			Id				aId)
		{
			return (uint32_t)aId >= 1 && (uint32_t)aId < (uint32_t)NUM_IDS;
		}

	}

}