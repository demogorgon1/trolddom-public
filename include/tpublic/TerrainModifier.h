#pragma once

namespace tpublic
{

	namespace TerrainModifier
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			FLAG_HUMIDITY,
			FLAG_TEMPERATURE,

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

			{ "humidity" },
			{ "temperature" },
		};

		static_assert(sizeof(INFO) / sizeof(Info) == NUM_IDS);

		inline constexpr const Info*
		GetInfo(
			Id				aId)
		{
			assert((uint32_t)aId < (uint32_t)NUM_IDS);
			return &INFO[aId];
		}

		inline constexpr bool
		ValidateId(
			Id				aId)
		{
			return (uint32_t)aId >= 1 && (uint32_t)aId < (uint32_t)NUM_IDS;
		}

		inline constexpr Id
		StringToId(
			const char*		aString)
		{
			std::string_view t(aString);
			for(uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				if(t == INFO[i].m_name)
					return (Id)i;
			}
			return INVALID_ID;
		}

	}

}