#include "../Pcheader.h"

#include <tpublic/DataErrorHandling.h>
#include <tpublic/UniformDistribution.h>

namespace tpublic::Test
{

	void
	TestUniformDistribution()
	{
		struct NotSoRandomGen
		{
			uint32_t
			operator()() const
			{
				return m_num;
			}

			uint32_t	m_num = 0;
		};

		UniformDistribution<uint32_t> distribution(100, 109);

		uint32_t counts[10] = { 0 };

		// Sample 1000 random numbers. Source random generator spread evenly across all 32 bits.
		// We should end up with exactly the same number of each value.
		uint64_t step = 1ULL + 0xFFFFFFFFULL / 1000ULL;
		for(uint64_t n = 0; n < 0x100000000ULL; n += step)
		{
			NotSoRandomGen random{(uint32_t)n};
			uint32_t i = distribution(random);

			TP_TEST(i >= 100 && i <= 109);
			counts[i - 100]++;
		}

		for(uint32_t i = 1; i < 10; i++)
			TP_TEST(counts[0] == counts[i]);
	}

}