#pragma once

#include "PerfTimer.h"

namespace tpublic
{

	class DebugPrintTimer
	{
	public:
		enum Resolution
		{
			RESOLUTION_MILLISECONDS,
			RESOLUTION_MICROSECONDS
		};

		DebugPrintTimer(
			const char*			aString,
			Resolution			aResolution = RESOLUTION_MILLISECONDS)
			: m_string(aString)
			, m_resolution(aResolution)
		{

		}

		~DebugPrintTimer()
		{
			uint64_t elapsed = m_perfTimer.GetElapsedMicroseconds();
			const char* unit = "";

			switch(m_resolution)
			{
			case RESOLUTION_MILLISECONDS:	unit = "milliseconds"; elapsed /= 1000ULL; break;
			case RESOLUTION_MICROSECONDS:	unit = "microseconds"; break;
			default:						break;
			}
			printf("'%s' took %zu %s.\n", m_string, elapsed, unit);
		}
	
	private:

		const char*	m_string = NULL;
		Resolution	m_resolution = RESOLUTION_MILLISECONDS;
		PerfTimer	m_perfTimer;
	};
	
}