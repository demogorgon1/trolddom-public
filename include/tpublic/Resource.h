#pragma once

namespace tpublic
{

	namespace Resource
	{

		enum Id : uint32_t
		{
			INVALID_ID,

			ID_HEALTH,
			ID_MANA,
			ID_RAGE,
			ID_ENERGY,

			NUM_IDS
		};

		enum Flag : uint8_t
		{
			FLAG_DEFAULT_TO_MAX = 0x01
		};

		struct Info
		{
			const char*		m_name;
			const char*		m_capitalizedName;
			uint8_t			m_flags;
			uint32_t		m_resurrectValue;
			uint8_t			m_colorR;
			uint8_t			m_colorG;
			uint8_t			m_colorB;
			bool			m_publicCombatLogEvents;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{			
			{ NULL, NULL, 0, 0, 0, 0, 0, false },

			{ "health",	"Health",	FLAG_DEFAULT_TO_MAX,	1, 0, 220, 0, true },		
			{ "mana",	"Mana",		FLAG_DEFAULT_TO_MAX,	0, 0, 0, 220, true },		
			{ "rage",	"Rage",		0,						0, 220, 0, 0, false },		
			{ "energy",	"Energy",	FLAG_DEFAULT_TO_MAX,    0, 220, 220, 0, false }
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