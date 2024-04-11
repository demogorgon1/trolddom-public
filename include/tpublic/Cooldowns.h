#pragma once

namespace
{
	inline std::string
	_FormatDateTime(
		time_t	aTimeStamp)
	{
		struct tm x;
		memset(&x, 0, sizeof(x));
		localtime_s(&x, &aTimeStamp);
		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "%d-%02d-%02d %02u:%02u:%02u", x.tm_year + 1900, x.tm_mon + 1, x.tm_mday, x.tm_hour, x.tm_min, x.tm_sec);
		return buffer;
	}
}

namespace tpublic
{

	namespace Data
	{
		struct Ability;
	}

	class IReader;
	class IWriter;
	class Manifest;

	class Cooldowns
	{
	public:
		struct Entry
		{
			void			
			ToStream(
				IWriter*							aStream) const
			{
				aStream->WriteUInt(m_cooldownId);
				
				if(aStream->IsNetworkStream())
				{
					aStream->WriteInt(m_start);
					aStream->WriteInt(m_end);
				}
				else
				{
					// Convert ticks into global UTC time stamps
					int32_t currentTick = aStream->GetTick();
					uint64_t currentTimeStamp = (uint64_t)time(NULL);

					uint64_t startTimeStamp = currentTimeStamp;

					if (m_start < currentTick)
						startTimeStamp -= (uint64_t)(currentTick - m_start) / 10;

					aStream->WriteUInt(startTimeStamp);

					uint64_t endTimeStamp = 0;

					if (m_end > m_start)
						endTimeStamp = startTimeStamp + (uint64_t)(m_end - m_start) / 10;

					aStream->WriteUInt(endTimeStamp);

					if(m_cooldownId == 2)
					{
						printf("WRITE: current: %u @ [%s] start: [%s] end: [%s]\n", currentTick, 
							_FormatDateTime((time_t)currentTimeStamp).c_str(),
							_FormatDateTime((time_t)startTimeStamp).c_str(),
							_FormatDateTime((time_t)endTimeStamp).c_str());
					}
				}
			}
			
			bool			
			FromStream(
				IReader*							aStream)
			{
				if (!aStream->ReadUInt(m_cooldownId))
					return false;

				if (aStream->IsNetworkStream())
				{
					if(!aStream->ReadInt(m_start))
						return false;
					if (!aStream->ReadInt(m_end))
						return false;
				}
				else
				{
					// Convert global UTC time stamps into ticks
					uint64_t startTimeStamp;
					uint64_t endTimeStamp;

					if (!aStream->ReadUInt(startTimeStamp))
						return false;
					if (!aStream->ReadUInt(endTimeStamp))
						return false;

					int32_t currentTick = aStream->GetTick();
					uint64_t currentTimeStamp = (uint64_t)time(NULL);

					if (startTimeStamp < currentTimeStamp)
						m_start = currentTick - (int32_t)((currentTimeStamp - startTimeStamp) * 10);
					else
						m_start = currentTick;

					if (endTimeStamp != 0)
					{
						assert(endTimeStamp >= startTimeStamp);
						m_end = m_start + (int32_t)((endTimeStamp - startTimeStamp) * 10);
					}
					else
					{
						m_end = 0;
					}

					if (m_cooldownId == 2)
					{
						printf("READ: current: %u @ [%s] start: %u @ [%s] end: %u @ [%s]\n", currentTick,
							_FormatDateTime((time_t)currentTimeStamp).c_str(),
							m_start,
							_FormatDateTime((time_t)startTimeStamp).c_str(),
							m_end,
							_FormatDateTime((time_t)endTimeStamp).c_str());
					}
				}

				return true;
			}

			// Public data
			uint32_t			m_cooldownId = 0;
			int32_t				m_start = 0;
			int32_t				m_end = 0;
		};

						Cooldowns();
						~Cooldowns();		

		bool			Update(
							int32_t					aTick);
		void			AddAbility(
							const Manifest*			aManifest,
							const Data::Ability*	aAbility,
							int32_t					aTick);
		bool			IsAbilityOnCooldown(
							const Data::Ability*	aAbility) const;
		void			ToStream(
							IWriter*				aStream) const;
		bool			FromStream(
							IReader*				aStream);

		// Public data
		std::vector<Entry>		m_entries;
	};

}