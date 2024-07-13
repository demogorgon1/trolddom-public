#pragma once

namespace tpublic
{

	namespace MoveSpeed
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_VERY_SLOW,
			ID_SLOW,
			ID_NORMAL,
			ID_FAST,

			NUM_IDS
		};
		
		struct Info
		{
			const char*		m_name;
			int32_t			m_tickBias;
			uint32_t		m_moveTime;
			uint32_t		m_moveCooldown;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{			
			{ NULL, 0, 0, 0 },

			{ "very_slow",	6,	400, 500 },
			{ "slow",		4,  300, 400 },
			{ "normal",		0,	200, 300 },
			{ "fast",		-1, 150, 200 }
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
			const char*		aString)
		{
			std::string_view s(aString);
			for(uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				const Info* t = &INFO[i];
				if(s == t->m_name)
					return (Id)i;
			}
			return INVALID_ID;
		}

	}

}