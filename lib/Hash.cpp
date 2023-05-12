#include "Pcheader.h"

#include <tpublic/Hash.h>

namespace tpublic
{

	namespace Hash
	{

		void
		MurmurHash2_160(
			const void*				aBuffer,
			size_t					aBufferSize,
			uint32_t				aSeed,
			uint32_t				aOutHash[5])
		{
			const uint8_t* data = (const uint8_t*)aBuffer;

			const uint32_t m1 = 0x5bd1e995;
			const uint32_t m2 = 0x19D699A5;
			const uint32_t m3 = 0xB11924E1;
			const uint32_t m4 = 0x16118B03;
			const uint32_t m5 = 0x53C93455;
			const uint32_t r = 24; 
        
			uint32_t x = (uint32_t)((aBufferSize >> 32) ^ (aBufferSize & 0xffffffff));
 
			uint32_t h1 = aSeed ^ x; 
			uint32_t h2 = aSeed ^ x ^ m2;
			uint32_t h3 = aSeed ^ x ^ m3;
			uint32_t h4 = aSeed ^ x ^ m4;
			uint32_t h5 = aSeed ^ x ^ m5; 
 
			while(aBufferSize >= 4)
			{
				uint32_t k1 = *((uint32_t*)data);

				k1 *= m1;
				k1 ^= k1 >> r;
				k1 *= m1; 

				h1 *= m1; h1 ^= k1; h1 ^= h2;
				h2 *= m2; h2 ^= k1; h2 ^= h3;
				h3 *= m3; h3 ^= k1; h3 ^= h4;
				h4 *= m4; h4 ^= k1; h4 ^= h5;
				h5 *= m5; h5 ^= k1; h5 ^= h1; 

				data += 4;
				aBufferSize -= 4;
			} 
 
			switch(aBufferSize)
			{
			case 3: h1 ^= data[2] << 16; h2 ^= data[2] << 16; h3 ^= data[2] << 16; h4 ^= data[2] << 16; h5 ^= data[2] << 16;
			case 2: h1 ^= data[1] << 8 ; h2 ^= data[1] << 8 ; h3 ^= data[1] << 8 ; h4 ^= data[1] << 8 ; h5 ^= data[1] << 8 ;
			case 1: h1 ^= data[0]      ; h2 ^= data[0]      ; h3 ^= data[0]      ; h4 ^= data[0]      ; h5 ^= data[0]      ;
					h1 *= m1           ; h2 *= m2           ; h3 *= m3           ; h4 *= m4           ; h5 *= m5           ;
			}; 
 
			h1 ^= h1 >> 13; h1 *= m1; h1 ^= h1 >> 15;
			h2 ^= h2 >> 13; h2 *= m2; h2 ^= h2 >> 15;
			h3 ^= h3 >> 13; h3 *= m3; h3 ^= h3 >> 15;
			h4 ^= h4 >> 13; h4 *= m4; h4 ^= h4 >> 15;
			h5 ^= h5 >> 13; h5 *= m5; h5 ^= h5 >> 15; 
 
			aOutHash[0] = h1 ^ h2 ^ h3 ^ h4 ^ h5;
			aOutHash[1] = h2 ^ h1;
			aOutHash[2] = h3 ^ h1;
			aOutHash[3] = h4 ^ h1;
			aOutHash[4] = h5 ^ h1;
		}

	}
			
}
