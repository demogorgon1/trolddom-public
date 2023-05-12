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
			uint8_t			m_colorR;
			uint8_t			m_colorG;
			uint8_t			m_colorB;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{			
			{ NULL, NULL },

			{ "health",	"Health",	FLAG_DEFAULT_TO_MAX,	0, 220, 0 },
			{ "mana",	"Mana",		FLAG_DEFAULT_TO_MAX,	0, 0, 220 },
			{ "rage",	"Rage",		0,						220, 0, 0 },
			{ "energy",	"Energy",	FLAG_DEFAULT_TO_MAX,    220, 220, 0 }
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
			for(uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				const Info* t = &INFO[i];
				if(strcmp(t->m_name, aString) == 0)
					return (Id)i;
			}
			return INVALID_ID;
		}

	}

}