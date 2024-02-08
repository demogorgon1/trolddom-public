#pragma once

namespace tpublic
{

	namespace ProfessionType
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_PRIMARY,
			ID_SECONDARY,

			NUM_IDS
		};

		struct Info
		{
			const char* m_name;
			const char* m_displayName;
			uint32_t	m_maxCount;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL, NULL, 0 },

			{ "primary",	"Primary",		2 },
			{ "secondary",	"Secondary",	UINT32_MAX }
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

		inline constexpr bool
		Validate(
			Id				aId)
		{
			return (uint32_t)aId < (uint32_t)NUM_IDS && aId != INVALID_ID;
		}

	}

}