#pragma once

namespace tpublic
{

	namespace DialogueScript
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_NONE,
			ID_CREATE_GUILD,

			NUM_IDS
		};

		struct Info
		{
			const char* m_name;
			const char* m_displayName;			
			uint8_t		m_flags;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL },

			{ "none" },
			{ "create_guild" }
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