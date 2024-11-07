#include "Pcheader.h"

#include <tpublic/Hash.h>
#include <tpublic/IReader.h>
#include <tpublic/IWriter.h>
#include <tpublic/TimeSeed.h>

namespace tpublic
{

	namespace
	{
		struct TimeAndDate
		{
			void
			FromTimeStamp(
				uint64_t	aTimeStamp) 
			{
				struct tm x;
				memset(&x, 0, sizeof(x));
				time_t t = (time_t)aTimeStamp;
				#if defined(WIN32)
					gmtime_s(&x, &t);
				#else
					gmtime_r(&t, &x);
				#endif
				m_year = (uint32_t)x.tm_year;
				m_month = (uint32_t)x.tm_mon;
				m_week = (uint32_t)x.tm_yday / 7;
				m_day = (uint32_t)x.tm_mday;
				m_hour = (uint32_t)x.tm_hour;
				m_minute = (uint32_t)x.tm_min;
			}

			uint64_t
			GetPeriodTimeStamp(
				TimeSeed::Type	aTimeSeedType) const
			{
				struct tm x;
				memset(&x, 0, sizeof(x));

				switch(aTimeSeedType)
				{
				case TimeSeed::TYPE_MINUTELY:
					x.tm_year = (int)m_year;
					x.tm_mon = (int)m_month;
					x.tm_mday = (int)m_day;
					x.tm_hour = (int)m_hour;
					x.tm_min = (int)m_minute;
					break;

				case TimeSeed::TYPE_HOURLY:
					x.tm_year = (int)m_year;
					x.tm_mon = (int)m_month;
					x.tm_mday = (int)m_day;
					x.tm_hour = (int)m_hour;
					break;

				case TimeSeed::TYPE_DAILY:
					x.tm_year = (int)m_year;
					x.tm_mon = (int)m_month;
					x.tm_mday = (int)m_day;
					break;

				case TimeSeed::TYPE_WEEKLY:
					x.tm_year = (int)m_year;
					x.tm_yday = (int)m_week * 7;
					break;
				case TimeSeed::TYPE_MONTHLY:
					x.tm_year = (int)m_year;
					x.tm_mon = (int)m_month;
					break;

				case TimeSeed::TYPE_YEARLY:
					x.tm_year = (int)m_year;
					break;

				default:
					assert(false);
					break;
				}

				#if defined(WIN32)
					return (uint64_t)_mkgmtime(&x);
				#else
					return (uint64_t)timegm(&x);
				#endif
			}

			uint64_t
			GetNextPeriodTimeStamp(
				TimeSeed::Type	aTimeSeedType) const
			{
				struct tm x;
				memset(&x, 0, sizeof(x));

				switch(aTimeSeedType)
				{
				case TimeSeed::TYPE_MINUTELY:	return GetPeriodTimeStamp(aTimeSeedType) + 60;
				case TimeSeed::TYPE_HOURLY:		return GetPeriodTimeStamp(aTimeSeedType) + 60 * 60;
				case TimeSeed::TYPE_DAILY:		return GetPeriodTimeStamp(aTimeSeedType) + 60 * 60 * 24;
				case TimeSeed::TYPE_WEEKLY:		return GetPeriodTimeStamp(aTimeSeedType) + 60 * 60 * 24 * 7;

				case TimeSeed::TYPE_MONTHLY:
					if(m_month == 11)
					{
						x.tm_year = (int)m_year + 1;
						x.tm_mon = 0;
					}
					else
					{
						x.tm_mon = (int)m_month + 1;
					}
					break;

				case TimeSeed::TYPE_YEARLY:
					x.tm_year = (int)m_year + 1;
					break;

				default:
					assert(false);
					break;
				}

				return (uint64_t)mktime(&x);
			}

			void
			DebugPrint() const
			{
				printf("%u-%02u-%02u (week %u, hour %u, minute %u)\n", m_year + 1900, m_month + 1, m_day, m_week + 1, m_hour, m_minute);
			}

			// Public data
			uint32_t	m_year = 0;
			uint32_t	m_month = 0;
			uint32_t	m_week = 0;
			uint32_t	m_day = 0;
			uint32_t	m_hour = 0;
			uint32_t	m_minute = 0;
		};

	}

	//----------------------------------------------------------------------------------------------

	TimeSeed::Type	
	TimeSeed::StringToType(
		const char*			aString)
	{
		std::string_view t(aString);
		if (t == "minutely")
			return TYPE_MINUTELY;
		if (t == "hourly")
			return TYPE_HOURLY;
		if (t == "bidaily")
			return TYPE_BIDAILY;
		if (t == "daily")
			return TYPE_DAILY;
		if (t == "weekly")
			return TYPE_WEEKLY;
		if (t == "monthly")
			return TYPE_MONTHLY;
		if (t == "yearly")
			return TYPE_YEARLY;
		return INVALID_TYPE;
	}

	uint64_t	
	TimeSeed::GetNextTimeStamp(
		uint64_t			aCurrentTimeStamp,
		Type				aType)
	{
		if (aType == TYPE_BIDAILY)
		{
			uint64_t periodDuration = 12 * 60 * 60;
			return (aCurrentTimeStamp / periodDuration) * periodDuration + periodDuration;
		}

		TimeAndDate currentTimeAndDate;
		currentTimeAndDate.FromTimeStamp(aCurrentTimeStamp);
		return currentTimeAndDate.GetNextPeriodTimeStamp(aType);
	}

	//----------------------------------------------------------------------------------------------

	void		
	TimeSeed::ToStream(
		IWriter*			aWriter) const
	{
		aWriter->WritePOD(m_type);
		aWriter->WritePOD(m_seed);
		aWriter->WriteUInt(m_fromTimeStamp);
		aWriter->WriteUInt(m_toTimeStamp);
	}
	
	bool		
	TimeSeed::FromStream(
		IReader*			aReader)
	{
		if (!aReader->ReadPOD(m_type))
			return false;
		if (!aReader->ReadPOD(m_seed))
			return false;
		if (!aReader->ReadUInt(m_fromTimeStamp))
			return false;
		if (!aReader->ReadUInt(m_toTimeStamp))
			return false;
		return true;
	}
	
	void		
	TimeSeed::Update(
		Type				aType,
		uint64_t			aCurrentTimeStamp)
	{
		m_type = aType;

		if(m_type == TYPE_BIDAILY)
		{
			uint64_t periodDuration = 12 * 60 * 60;
			
			m_fromTimeStamp = (aCurrentTimeStamp / periodDuration) * periodDuration;
			m_toTimeStamp = m_fromTimeStamp + periodDuration;
		}
		else
		{
			TimeAndDate currentTimeAndDate;
			currentTimeAndDate.FromTimeStamp(aCurrentTimeStamp);

			m_fromTimeStamp = currentTimeAndDate.GetPeriodTimeStamp(m_type);
			m_toTimeStamp = currentTimeAndDate.GetNextPeriodTimeStamp(m_type);
		}

		uint64_t hash = Hash::Splitmix_64(m_fromTimeStamp) ^ Hash::Splitmix_64(m_toTimeStamp);
		m_seed = (uint32_t)((0xFFFFFFFFULL & hash) ^ (hash > 32ULL));

		// Handle the very unlikely situation we get a seed of 0. Can't have that as it means "no seed".
		if(m_seed == 0)
			m_seed = 1;
	}

	bool		
	TimeSeed::operator==(
		const TimeSeed&		aOther) const
	{
		return m_type == aOther.m_type && m_seed == aOther.m_seed && m_fromTimeStamp == aOther.m_fromTimeStamp && m_toTimeStamp == aOther.m_toTimeStamp;
	}

	bool		
	TimeSeed::operator!=(
		const TimeSeed&		aOther) const
	{
		return m_type != aOther.m_type || m_seed != aOther.m_seed || m_fromTimeStamp != aOther.m_fromTimeStamp || m_toTimeStamp != aOther.m_toTimeStamp;
	}

	bool		
	TimeSeed::IsSet() const
	{
		return m_type != INVALID_TYPE;
	}

}