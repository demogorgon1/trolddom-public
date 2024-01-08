#pragma once

namespace tpublic
{

	namespace ItemBinding
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_NEVER,
			ID_WHEN_EQUIPPED,
			ID_WHEN_PICKED_UP,

			NUM_IDS
		};

		struct Info
		{
			const char* m_name;
			const char* m_displayName;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL, NULL },

			{ "never",					"" },
			{ "when_equipped",			"Binds when equipped" },
			{ "when_picked_up",			"Binds when picked up" }
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
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				const Info* t = &INFO[i];
				if (strcmp(t->m_name, aString) == 0)
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