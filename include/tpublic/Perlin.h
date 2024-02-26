#pragma once

namespace tpublic
{

	namespace Perlin
	{
		
		struct Seed
		{
			int32_t		m_permutations[512];
		};

		void			InitSeed(
							Seed*			aSeed,
							std::mt19937&	aRandom);		
		int32_t			Sample(
							int32_t			aX,
							int32_t			aY,
							int32_t			aZ,
							const Seed*		aSeed = NULL);

	}

}