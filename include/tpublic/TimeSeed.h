#pragma once

namespace tpublic
{

	class IReader;
	class IWriter;

	class TimeSeed
	{
	public:
		enum Type : uint8_t
		{
			INVALID_TYPE,

			TYPE_MINUTELY,
			TYPE_HOURLY,
			TYPE_DAILY,
			TYPE_WEEKLY,
			TYPE_MONTHLY,
			TYPE_YEARLY
		};

		static Type	StringToType(
						const char*			aString);

		void		ToStream(
						IWriter*			aWriter) const;
		bool		FromStream(
						IReader*			aReader);
		void		Update(
						Type				aType,
						uint64_t			aCurrentTimeStamp);
		bool		operator==(
						const TimeSeed&		aOther) const;
		bool		operator!=(
						const TimeSeed&		aOther) const;
		bool		IsSet() const;

		// Public data
		Type				m_type = INVALID_TYPE;
		uint32_t			m_seed = 0;
		uint64_t			m_fromTimeStamp = 0;
		uint64_t			m_toTimeStamp = 0;
	};

}