#include "Pcheader.h"

#include <tpublic/DataErrorHandling.h>
#include <tpublic/Helpers.h>
#include <tpublic/Image.h>
#include <tpublic/ImageResampler.h>

namespace tpublic
{

	ImageResampler::ImageResampler()
		: m_target(NULL)
		, m_source(NULL)
		, m_sourceWidth(0)
		, m_sourceHeight(0)
		, m_targetWidth(0)
		, m_targetHeight(0)
		, m_flags(0)
	{

	}

	ImageResampler::~ImageResampler()
	{
		if(m_target != NULL)
			delete [] m_target;

		if(m_source != NULL)
			delete [] m_source;
	}

	void	
	ImageResampler::Init(
		uint32_t		aSourceWidth,
		uint32_t		aSourceHeight,
		uint32_t		aTargetWidth,
		uint32_t		aTargetHeight,
		uint32_t		aFlags)
	{
		assert(m_target == NULL);
		assert(m_source == NULL);
		TP_CHECK(aSourceWidth >= aTargetWidth && aSourceHeight >= aTargetHeight, "Only downsampling is supported.");

		m_target = new TargetPixel[aTargetWidth * aTargetHeight];

		m_sourceWidth = aSourceWidth;
		m_sourceHeight = aSourceHeight;
		m_targetWidth = aTargetWidth;
		m_targetHeight = aTargetHeight;
		m_flags = aFlags;

		// Calculate maximum source sample area dimensions
		uint32_t maxSourceSampleAreaWidth = m_sourceWidth / m_targetWidth;
		if (m_sourceWidth % m_targetWidth != 0)
			maxSourceSampleAreaWidth++;
		maxSourceSampleAreaWidth++;

		uint32_t maxSourceSampleAreaHeight = m_sourceHeight / m_targetHeight;
		if (m_sourceHeight % m_targetHeight != 0)
			maxSourceSampleAreaHeight++;
		maxSourceSampleAreaHeight++;

		// Allocate space for source weights
		m_source = new uint32_t[maxSourceSampleAreaWidth * maxSourceSampleAreaHeight * m_targetWidth * m_targetHeight];

		// For each target pixel: determine what source pixels contribute to it and by how much
		TargetPixel* t = m_target;
		uint32_t* p = m_source;

		for(uint32_t y = 0; y < m_targetHeight; y++)
		{
			uint32_t sourceY0 = (y * m_sourceHeight) / m_targetHeight;
			uint32_t sourceY1 = ((y + 1) * m_sourceHeight) / m_targetHeight;

			uint32_t sourceSampleAreaHeight = sourceY1 - sourceY0;
			if (m_sourceHeight % m_targetHeight != 0)
				sourceSampleAreaHeight++;

			if (sourceY0 + sourceSampleAreaHeight == m_sourceHeight + 1)
				sourceSampleAreaHeight--;

			for(uint32_t x = 0; x < m_targetWidth; x++)
			{
				uint32_t sourceX0 = (x * m_sourceWidth) / m_targetWidth;
				uint32_t sourceX1 = ((x + 1) * m_sourceWidth) / m_targetWidth;

				t->m_sourceSampleAreaWidth = sourceX1 - sourceX0;
				if (m_sourceWidth % m_targetWidth != 0)
					t->m_sourceSampleAreaWidth++;

				if(sourceX0 + t->m_sourceSampleAreaWidth == m_sourceWidth + 1)
					t->m_sourceSampleAreaWidth--;

				t->m_sourceSampleAreaHeight = sourceSampleAreaHeight;
				t->m_firstWeight = p;

				for (uint32_t i = 0; i < t->m_sourceSampleAreaHeight; i++)
				{
					int64_t y0fp = (int64_t)(((int64_t)(sourceY0 + i) * FIXED_POINT_MULT * m_targetHeight) / (int64_t)m_sourceHeight) - ((int64_t)y) * FIXED_POINT_MULT;
					int64_t y1fp = y0fp + (int64_t)(m_targetHeight * FIXED_POINT_MULT) / (int64_t)m_sourceHeight;
					int64_t hfp = Helpers::Clamp<int64_t>(y1fp, 0, FIXED_POINT_MULT) - Helpers::Clamp<int64_t>(y0fp, 0, FIXED_POINT_MULT);

					for(uint32_t j = 0; j < t->m_sourceSampleAreaWidth; j++)
					{
						int64_t x0fp = (int64_t)(((int64_t)(sourceX0 + j) * FIXED_POINT_MULT * m_targetWidth) / (int64_t)m_sourceWidth) - ((int64_t)x) * FIXED_POINT_MULT;
						int64_t x1fp = x0fp + (int64_t)(m_targetWidth * FIXED_POINT_MULT) / (int64_t)m_sourceWidth;
						int64_t wfp = Helpers::Clamp<int64_t>(x1fp, 0, FIXED_POINT_MULT) - Helpers::Clamp<int64_t>(x0fp, 0, FIXED_POINT_MULT);

						int64_t weightfp = (hfp * wfp) / FIXED_POINT_MULT;

						*p = (uint32_t)weightfp;

						p++;
					}
				}

				t++;
			}
		}

		if(m_flags & FLAG_FORCE_POWER_OF_TWO)
		{
			m_paddedTargetWidth = Helpers::NearestGreaterOrEqualPowerOfTwo<uint32_t>(m_targetWidth);
			m_paddedTargetHeight = Helpers::NearestGreaterOrEqualPowerOfTwo<uint32_t>(m_targetHeight);
		}
		else
		{
			m_paddedTargetWidth = m_targetWidth;
			m_paddedTargetHeight = m_targetHeight;
		}
	}

	void 
	ImageResampler::Process(
		const Image*			aIn,
		Image*					aOut) const
	{
		assert(m_source != NULL);
		TP_CHECK(aIn->GetWidth() == m_sourceWidth && aIn->GetHeight() == m_sourceHeight, "Image size doesn't match resampler.");

		aOut->Allocate(m_paddedTargetWidth, m_paddedTargetHeight);

		_ProcessUInt8(
			(const uint8_t*)aIn->GetData(), 
			4, 
			m_paddedTargetWidth - m_targetWidth, 
			m_paddedTargetHeight - m_targetHeight,
			(uint8_t*)aOut->GetData());
	}

	//---------------------------------------------------------------------------------------------------

	void		
	ImageResampler::_ProcessUInt8(
		const uint8_t*		aIn,
		uint32_t			aComponentCount,
		uint32_t			aOutputRightPadding,
		uint32_t			aOutputBottomPadding,
		uint8_t*			aOut) const
	{
		assert(m_targetWidth > 0);
		assert(m_targetHeight > 0);
			
		uint8_t* outP = aOut;
		const TargetPixel* target = m_target;

		for(uint32_t y = 0; y < m_targetHeight; y++)
		{
			uint32_t sourceY0 = (y * m_sourceHeight) / m_targetHeight;

			for(uint32_t x = 0; x < m_targetWidth; x++)
			{
				uint32_t sum[4] = {0, 0, 0, 0};

				uint32_t sourceX0 = (x * m_sourceWidth) / m_targetWidth;
				uint32_t sourceOffset = sourceX0 + sourceY0 * m_sourceWidth;

				const uint32_t* weight = target->m_firstWeight;

				for (uint32_t i = 0; i < target->m_sourceSampleAreaHeight; i++)
				{
					const uint8_t* sourceP = aIn + sourceOffset * aComponentCount;

					for (uint32_t j = 0; j < target->m_sourceSampleAreaWidth; j++)
					{
						uint32_t w = *weight;		
						weight++;

						for(uint32_t k = 0; k < aComponentCount; k++)
						{
							sum[k] += (uint32_t)(*sourceP) * w;
							sourceP++;
						}
					}

					sourceOffset += m_sourceWidth;
				}

				for(uint32_t k = 0; k < aComponentCount; k++)
					outP[k] = (uint8_t)(sum[k] / (uint32_t)FIXED_POINT_MULT);

				outP += aComponentCount;
				target++;
			}

			if(aOutputRightPadding > 0)
			{
				// Add padding to the right. First padded pixel should be equal to the last unpadded one, to avoid texture sampling artifacts.
				memcpy(outP, outP - aComponentCount, aComponentCount);
				outP += aComponentCount;

				memset(outP, 255, (aOutputRightPadding - 1) * aComponentCount);
				outP += (aOutputRightPadding - 1) * aComponentCount;
			}
		}

		if(aOutputBottomPadding > 0)
		{
			// Add padding to the bottom, similar to the right. Again, first padded row should be equal to the last unpadded one.
			memcpy(outP, outP - aComponentCount * m_paddedTargetWidth, aComponentCount * m_paddedTargetWidth);
			outP += aComponentCount * m_paddedTargetWidth;

			memset(outP, 255, (aOutputBottomPadding - 1) * aComponentCount * m_paddedTargetWidth);
			outP += (aOutputBottomPadding - 1) * aComponentCount * m_paddedTargetWidth;
		}
	}

}
