#pragma once

namespace tpublic
{

	class Image;

	class ImageResampler
	{
	public:
		static const uint32_t FLAG_FORCE_POWER_OF_TWO = 0x1;

					ImageResampler();
					~ImageResampler();

		void		Init(
						uint32_t			aSourceWidth,
						uint32_t			aSourceHeight,
						uint32_t			aTargetWidth,
						uint32_t			aTargetHeight,
						uint32_t			aFlags);
		void		Process(
						const Image*		aIn,
						Image*				aOut) const;

	private:

		static const int64_t FIXED_POINT_MULT = 16383;

		uint32_t				m_sourceWidth;
		uint32_t				m_sourceHeight;
		uint32_t				m_targetWidth;
		uint32_t				m_targetHeight;

		struct TargetPixel
		{
			uint32_t			m_sourceSampleAreaWidth;
			uint32_t			m_sourceSampleAreaHeight;
			const uint32_t*		m_firstWeight;
		};

		uint32_t*				m_source;
		TargetPixel*			m_target;

		uint32_t				m_paddedTargetWidth;
		uint32_t				m_paddedTargetHeight;
		uint32_t				m_flags;

		void		_ProcessUInt8(
						const uint8_t*		aIn,
						uint32_t			aComponentCount,
						uint32_t			aOutputRightPadding,
						uint32_t			aOutputBottomPadding,
						uint8_t*			aOut) const;
	};

}