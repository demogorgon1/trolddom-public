#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "Vec2.h"

namespace tpublic
{	

	class PackedDistanceField
	{
	public:
		enum Encoding : uint8_t
		{
			ENCODING_4_BITS,
			ENCODING_8_BITS,
			ENCODING_16_BITS,
			ENCODING_32_BITS,

			NUM_ENCODINGS
		};

		struct EncodingInfo
		{
			uint32_t	m_bits = 0;
			uint32_t	m_mask = 0;
			uint32_t	m_distancesPerElement = 0;
		};

		static inline const EncodingInfo ENCODING_INFO[NUM_ENCODINGS] =
		{
			{ 4,  0xF, 8 },
			{ 8,  0xFF, 4 },
			{ 16, 0xFFFF, 2 },
			{ 32, 0xFFFFFFFF, 1 }
		};

		PackedDistanceField()
		{

		}

		~PackedDistanceField()
		{
			if(m_elems != NULL)
				delete [] m_elems;
		}

		void
		CopyFrom(
			const PackedDistanceField*	aOther)
		{
			assert(m_elems == NULL);
			m_elemCount = aOther->m_elemCount;
			m_size = aOther->m_size;
			m_encoding = aOther->m_encoding;
			m_elems = new uint32_t[m_elemCount];
			memcpy(m_elems, aOther->m_elems, sizeof(uint32_t) * m_elemCount);
		}

		void
		ToStream(
			IWriter*					aWriter) const
		{
			assert(m_elems != NULL);
			m_size.ToStream(aWriter);
			aWriter->WritePOD(m_encoding);
			aWriter->WriteUInt(m_elemCount);
			aWriter->Write(m_elems, m_elemCount * sizeof(uint32_t));
		}

		bool
		FromStream(
			IReader*					aReader) 
		{
			assert(m_elems == NULL);
			if(!m_size.FromStream(aReader))
				return false;
			if (!aReader->ReadPOD(m_encoding))
				return false;
			if (!aReader->ReadUInt(m_elemCount))
				return false;
			
			uint32_t bytes = m_elemCount * sizeof(uint32_t);
			if(bytes > 1024 * 1024)
				return false;

			m_elems = new uint32_t[m_elemCount];

			if(aReader->Read(m_elems, bytes) != bytes)
				return false;

			return true;
		}

		void
		DebugPrint() const
		{
			for(int32_t y = 0; y < m_size.m_y; y++)
			{
				for (int32_t x = 0; x < m_size.m_x; x++)
				{
					uint32_t v = Get({ x, y });
					if(v != UINT32_MAX)
						printf("%-4u", v);
					else
						printf("    ");
				}
				printf("\n");
			}
		}

		void		
		Create(
			const uint32_t*				aDistanceField,
			const Vec2&					aSize)
		{
			assert(aSize.m_x > 0 && aSize.m_y > 0);
			assert(m_elems == NULL);
			
			m_size = aSize;

			uint32_t distanceCount = (uint32_t)(m_size.m_x * m_size.m_y);
			uint32_t maxDistance = 0;

			for(uint32_t i = 0; i < distanceCount; i++)
			{		
				uint32_t d = aDistanceField[i];
				if(d != UINT32_MAX && d > maxDistance)
					maxDistance = d;
			}
			
			if(maxDistance < 0xF)
				m_encoding = ENCODING_4_BITS;
			else if (maxDistance < 0xFF)
				m_encoding = ENCODING_8_BITS;
			else if (maxDistance < 0xFFFF)
				m_encoding = ENCODING_16_BITS;
			else
				m_encoding = ENCODING_32_BITS;

			const EncodingInfo* encodingInfo = &ENCODING_INFO[m_encoding];

			m_elemCount = distanceCount / encodingInfo->m_distancesPerElement;
			if(distanceCount % encodingInfo->m_distancesPerElement)
				m_elemCount++;

			m_elems = new uint32_t[m_elemCount];
			memset(m_elems, 0, m_elemCount * sizeof(uint32_t));

			uint32_t elemOffset = 0;
			uint32_t bitOffset = 0;

			for(uint32_t i = 0; i < distanceCount; i++)
			{
				uint32_t v = aDistanceField[i];
				
				if(v == UINT32_MAX)
					v = 0;
				else
					v++;

				assert(v <= encodingInfo->m_mask);

				assert(elemOffset < m_elemCount);
				m_elems[elemOffset] |= v << bitOffset;

				bitOffset += encodingInfo->m_bits;

				if(bitOffset == 32)
				{
					bitOffset = 0;
					elemOffset++;
				}
			}
		}

		uint32_t
		Get(
			const Vec2&					aPosition) const
		{
			assert(m_elems != NULL);
			assert((uint32_t)m_encoding < (uint32_t)NUM_ENCODINGS);
			const EncodingInfo* encodingInfo = &ENCODING_INFO[m_encoding];

			if(aPosition.m_x >= 0 && aPosition.m_y >= 0 && aPosition.m_x < m_size.m_x && aPosition.m_y < m_size.m_y)
			{
				uint32_t i = aPosition.m_x + aPosition.m_y * m_size.m_x;

				uint32_t elemOffset = i / encodingInfo->m_distancesPerElement;
				uint32_t bitOffset = (i % encodingInfo->m_distancesPerElement) * encodingInfo->m_bits;

				assert(elemOffset < m_elemCount);

				uint32_t value = ((m_elems[elemOffset] >> bitOffset) & encodingInfo->m_mask);

				if(value == 0)
					return UINT32_MAX;

				return value - 1;
			}

			return UINT32_MAX;
		}
	
		// Public data
		uint32_t*	m_elems = NULL;
		uint32_t	m_elemCount = 0;
		Vec2		m_size;		
		Encoding	m_encoding = Encoding(0);
	};

}