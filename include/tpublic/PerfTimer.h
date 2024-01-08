#pragma once

namespace tpublic
{

	class PerfTimer
	{
	public:
		PerfTimer() noexcept
		{
			m_startTime = std::chrono::high_resolution_clock::now();
		}

		~PerfTimer()
		{

		}

		uint64_t
		GetElapsedMilliseconds() const noexcept
		{
			std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
			return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_startTime).count();
		}

		uint64_t
		GetElapsedMicroseconds() const noexcept
		{
			std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
			return (uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_startTime).count();
		}

		void
		Reset() noexcept
		{
			m_startTime = std::chrono::high_resolution_clock::now();
		}
		
	private:

		std::chrono::time_point<std::chrono::high_resolution_clock>		m_startTime;	
	};

}
