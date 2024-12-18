#pragma once

#include "PerfTimer.h"

namespace tpublic
{

	class DebugPrintTimer
	{
	public:
		DebugPrintTimer(
			const char*			aString)
			: m_string(aString)
		{

		}

		~DebugPrintTimer()
		{
			uint64_t elapsedMilliseconds = m_perfTimer.GetElapsedMilliseconds();
			printf("'%s' took %zu ms.\n", m_string, elapsedMilliseconds);
		}
	
	private:

		const char*	m_string = NULL;
		PerfTimer	m_perfTimer;
	};
	
}