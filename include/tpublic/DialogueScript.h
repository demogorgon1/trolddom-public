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
			ID_UNTRAIN_TALENTS,
			ID_TRIGGER,

			NUM_IDS
		};

		struct Info
		{
			const char* m_name;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL },

			{ "none" },
			{ "create_guild" },
			{ "untrain_talents" },
			{ "trigger" }
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