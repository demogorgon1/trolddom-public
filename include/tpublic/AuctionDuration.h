#pragma once

namespace tpublic
{

	namespace AuctionDuration
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_12_HOURS,
			ID_24_HOURS,
			ID_48_HOURS,
			ID_72_HOURS,

			NUM_IDS
		};

		struct Info
		{
			const char* m_displayName;
			int64_t		m_depositValuePercentage;
			uint32_t	m_seconds;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL, 0, 0 },

			{ "12 Hours", 2, 12 * 60 * 60 },
			{ "24 Hours", 4, 24 * 60 * 60 },
			{ "48 Hours", 6, 48 * 60 * 60 },
			{ "72 Hours", 8, 72 * 60 * 60 },
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

		inline constexpr int64_t
		CalculateDeposit(
			Id				aId,
			int64_t			aValue)
		{
			if(!ValidateId(aId))
				return 0;
			const Info* info = GetInfo(aId);
			return (aValue * info->m_depositValuePercentage) / 100;
		}

		inline constexpr time_t
		IdToSeconds(
			Id				aId)
		{
			if (!ValidateId(aId))
				return 0;
			return (time_t)(GetInfo(aId)->m_seconds);
		}

	}

}