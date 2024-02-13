#pragma once

namespace tpublic
{

	namespace Hash
	{

		struct CheckSum
		{
			CheckSum()
				: m_hash(0)
				, m_processedBytes(0)
			{

			}

			void	AddData(
						const void*				aBuffer,
						size_t					aBufferSize);
			bool	operator==(
						const CheckSum&			aOther) const;
			bool	operator!=(
						const CheckSum&			aOther) const;

			template <typename _T>
			void
			AddPOD(
				const _T&						aData)
			{
				AddData(&aData, sizeof(_T));
			}

			template <typename _T> 
			void
			AddPODVector(
				const std::vector<_T>&			aVector)
			{
				if(aVector.size() > 0)
					AddData(&aVector[0], sizeof(_T) * aVector.size());
			}

			// Public data
			uint32_t		m_hash;
			size_t			m_processedBytes;			
		};

		uint32_t	CRC_32(
						const void*				aBuffer,
						size_t					aBufferSize);
		void		MurmurHash2_160(
						const void*				aBuffer,
						size_t					aBufferSize,
						uint32_t				aSeed,
						uint32_t				aOutHash[5]);
		uint32_t	String(
						const char*				aString);
		bool		File(
						const char*				aPath,
						uint32_t				aOutHash[5]);
	
		inline constexpr uint64_t
		Splitmix_64(
			uint64_t							aValue)
		{
			uint64_t x = (aValue ^ (aValue >> 30ULL)) * 0xbf58476d1ce4e5b9ULL;
			x = (x ^ (x >> 27ULL)) * 0x94d049bb133111ebULL;
			x = x ^ (x >> 31ULL);
			return x;
		}

		inline constexpr uint64_t
		Splitmix_2_32(
			uint32_t							aValue1,
			uint32_t							aValue2)
		{
			uint64_t combined = ((uint64_t)aValue1 << 32ULL) | (uint64_t)aValue2;
			return Splitmix_64(combined);
		}

		template <typename _T>
		inline constexpr uint32_t
		PODVector_32(
			const std::vector<_T>&				aVector)
		{
			if(aVector.size() == 0)
				return 0;
		
			uint32_t hash[5];
			MurmurHash2_160(&aVector[0], sizeof(_T) * aVector.size(), 0x43025501, hash);
			return hash[0] ^ hash[1] ^ hash[2] ^ hash[3] ^ hash[4];
		}
	}
		
}
