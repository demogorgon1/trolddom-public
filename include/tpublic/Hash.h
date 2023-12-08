#pragma once

namespace tpublic
{

	namespace Hash
	{

		void		MurmurHash2_160(
						const void*				aBuffer,
						size_t					aBufferSize,
						uint32_t				aSeed,
						uint32_t				aOutHash[5]);
		uint32_t	String(
						const char*				aString);
	
		inline constexpr uint64_t
		Splitmix_64(
			uint64_t							aValue)
		{
			uint64_t x = (aValue ^ (aValue >> 30ULL)) * 0xbf58476d1ce4e5b9ULL;
			x = (x ^ (x >> 27ULL)) * 0x94d049bb133111ebULL;
			x = x ^ (x >> 31ULL);
			return x;
		}

	}
		
}
