#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	template <size_t _MaxCount>
	class EventHistory
	{
	public:
		static_assert(_MaxCount > 1);

		EventHistory(
			uint32_t				aMaxAge)
			: m_maxAge((uint64_t)aMaxAge)
		{
			
		}

		~EventHistory()
		{

		}

		void	
		AddEvent(
			uint64_t				aCurrentTimeStamp)
		{
			// Replace oldest timestamp
			uint64_t minTimeStamp = m_values[0];
			size_t minIndex = 0;

			for(size_t i = 1; i < _MaxCount; i++)
			{
				if(m_values[i] < minTimeStamp)
				{
					minIndex = i;
					minTimeStamp = m_values[i];
				}
			}

			m_values[minIndex] = aCurrentTimeStamp;
		}
		
		size_t	
		GetCount(
			uint64_t				aCurrentTimeStamp) const
		{
			size_t count = 0;

			for(size_t i = 0; i < _MaxCount; i++) 
			{
				uint64_t t = m_values[i];
				uint64_t age = aCurrentTimeStamp > t ? aCurrentTimeStamp - t : 0;
				if(age <= m_maxAge)
					count++;
			}

			return count;
		}

		void
		ToStream(
			IWriter*				aWriter) const
		{
			for(size_t i = 0; i < _MaxCount; i++)
				aWriter->WriteUInt(m_values[i]);
		}

		bool
		FromStream(
			IReader*				aReader) 
		{
			for(size_t i = 0; i < _MaxCount; i++)
			{
				if(!aReader->ReadUInt(m_values[i]))
					return false;
			}
			return true;
		}

	private:

		uint64_t					m_maxAge = 0;
		uint64_t					m_values[_MaxCount] = { 0 };
	};

}