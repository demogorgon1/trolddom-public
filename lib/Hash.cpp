#include "Pcheader.h"

#include <tpublic/Hash.h>
#include <tpublic/Singleton.h>

namespace tpublic
{

	namespace
	{
		
		struct _CRC32LookUpTable : public Singleton<_CRC32LookUpTable>
		{
			_CRC32LookUpTable()
			{
				for(uint32_t i = 0; i < 256; i++)
					m_table[i] = _CRC32ForByte(i);
			}

			uint32_t
			_CRC32ForByte(
				uint32_t		aByte)
			{
				uint32_t r = aByte;
				for (uint32_t j = 0; j < 8; j++)
					r = (r & 1 ? 0 : 0xEDB88320) ^ r >> 1;
				return r ^ 0xFF000000;
			}

			uint32_t	m_table[256];
		};

	}

	namespace Hash
	{

		void	
		CheckSum::AddData(
			const void*				aBuffer,
			size_t					aBufferSize)
		{
			const uint32_t* table = _CRC32LookUpTable::GetInstance()->m_table;
			const uint8_t* p = (const uint8_t*)aBuffer;

			for(size_t i = 0; i < aBufferSize; i++)
			{
				m_hash = table[(uint8_t)m_hash ^ *p] ^ m_hash >> 8;
				p++;
			}

			m_processedBytes += aBufferSize;
		}

		void	
		CheckSum::AddString(
			const char*				aString)
		{
			AddData(aString, strlen(aString));
		}

		bool	
		CheckSum::operator==(
			const CheckSum&			aOther) const
		{
			return m_hash == aOther.m_hash && m_processedBytes == aOther.m_processedBytes;
		}

		bool	
		CheckSum::operator!=(
			const CheckSum&			aOther) const
		{
			return m_hash != aOther.m_hash || m_processedBytes != aOther.m_processedBytes;
		}

		uint32_t
		CRC_32(
			const void*				aBuffer,
			size_t					aBufferSize)
		{
			CheckSum checkSum;
			checkSum.AddData(aBuffer, aBufferSize);
			return checkSum.m_hash;
		}

		uint32_t	
		String(
			const char*				aString)
		{
			uint32_t hash[5];
			MurmurHash2_160(aString, strlen(aString), 0x94812923, hash);
			return hash[0] ^ hash[1] ^ hash[2] ^ hash[3] ^ hash[4];
		}

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

		bool		
		File(
			const char*				aPath,
			uint32_t				aOutHash[5])
		{
			FILE* f = fopen(aPath, "rb");
			if(f == NULL)
				return false;

			bool ok = true;

			try
			{
				fseek(f, 0, SEEK_END);
				size_t fileSize = (size_t)ftell(f);
				fseek(f, 0, SEEK_SET);

				if(fileSize == 0)
				{
					memset(aOutHash, 0, sizeof(aOutHash));
				}
				else
				{
					std::vector<uint8_t> buffer;
					buffer.resize(fileSize);
					size_t readBytes = (size_t)fread(&buffer[0], 1, fileSize, f);

					if(readBytes == fileSize)
					{
						MurmurHash2_160(&buffer[0], fileSize, 0x941B9021, aOutHash);
					}
					else
					{
						ok = false;
					}
				}
			}
			catch(...)
			{
				fclose(f);
				return false;
			}

			fclose(f);
			return ok;
		}

	}
			
}
