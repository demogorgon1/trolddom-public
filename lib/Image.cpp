#include "Pcheader.h"

#include <lodepng/lodepng.h>

#include <kpublic/Image.h>

namespace kpublic
{

	Image::Image()
		: m_data(NULL)
		, m_width(0)
		, m_height(0)
	{

	}
	
	Image::Image(
		uint32_t		aWidth,
		uint32_t		aHeight)
		: m_data(NULL)
		, m_width(0)
		, m_height(0)
	{
		Allocate(aWidth, aHeight);
	}
	
	Image::~Image()
	{
		Release();
	}

	void		
	Image::Allocate(
		uint32_t		aWidth,
		uint32_t		aHeight)
	{
		if(m_data != NULL)
			free(m_data);

		// Using malloc() to match lodepng allocations
		m_data = (RGBA*)malloc(sizeof(RGBA) * aWidth * aHeight);
		m_width = aWidth;
		m_height = aHeight;
		memset(m_data, 0, GetSize());
	}
	
	void		
	Image::LoadPNG(
		const char*		aPath)
	{
		std::filesystem::path p = aPath;
		KP_CHECK(p.extension() == ".png", "Not a PNG: %s", aPath);

		Release();
		
		uint32_t result = (uint32_t)lodepng_decode32_file((unsigned char**)&m_data, (unsigned*)&m_width, (unsigned*)&m_height, aPath);
		KP_CHECK(result == 0, "Failed to load PNG: %s", aPath);
	}
	
	void		
	Image::SavePNG(
		const char*		aPath) const
	{
		std::filesystem::path p = aPath;
		KP_CHECK(p.extension() == ".png", "Not a PNG: %s", aPath);

		assert(HasData());
		assert(m_width > 0 && m_height > 0);

		uint32_t result = (uint32_t)lodepng_encode32_file(aPath, (const unsigned char*)m_data, (unsigned)m_width, (unsigned)m_height);
		KP_CHECK(result == 0, "Failed to save PNG: %s", aPath);
	}
	
	void		
	Image::Extract(
		uint32_t		aX,
		uint32_t		aY,
		uint32_t		aWidth,
		uint32_t		aHeight,
		Image&			aOut) const
	{
		assert(HasData());
		assert(aX + aWidth <= m_width && aY + aHeight <= m_height);

		aOut.Allocate(aWidth, aHeight);

		for(uint32_t y = 0; y < aHeight; y++)
			memcpy(aOut.GetData() + y * aWidth, GetData() + aX + (aY + y) * m_width, sizeof(RGBA) * aWidth);
	}
	
	void		
	Image::Insert(
		uint32_t		aX,
		uint32_t		aY,
		const Image&	aImage)
	{
		assert(HasData());
		assert(aImage.HasData());
		assert(aX + aImage.m_width <= m_width && aY + aImage.m_height <= m_height);

		for(uint32_t y = 0; y < aImage.m_height; y++)
			memcpy(GetData() + aX + (aY + y) * m_width, aImage.GetData() + y * aImage.GetWidth(), sizeof(RGBA) * aImage.GetWidth());
	}

	void		
	Image::Clear(
		const RGBA&		aColor)
	{
		assert(HasData());
		RGBA* p = m_data;
		for(uint32_t i = 0, count = m_width * m_height; i < count; i++)
			*p++ = aColor;
	}

	void		
	Image::DrawVerticalGradient(
		const RGBA&		aColorTop,
		const RGBA&		aColorBottom)
	{
		assert(HasData());
		RGBA* p = m_data;
		for(uint32_t y = 0; y < m_height; y++)
		{
			uint32_t r1 = ((uint32_t)aColorTop.m_r * (m_height - y)) / m_height;
			uint32_t g1 = ((uint32_t)aColorTop.m_g * (m_height - y)) / m_height;
			uint32_t b1 = ((uint32_t)aColorTop.m_b * (m_height - y)) / m_height;
			uint32_t a1 = ((uint32_t)aColorTop.m_a * (m_height - y)) / m_height;

			uint32_t r2 = ((uint32_t)aColorBottom.m_r * y) / m_height;
			uint32_t g2 = ((uint32_t)aColorBottom.m_g * y) / m_height;
			uint32_t b2 = ((uint32_t)aColorBottom.m_b * y) / m_height;
			uint32_t a2 = ((uint32_t)aColorBottom.m_a * y) / m_height;

			uint32_t r = r1 + r2;
			uint32_t g = g1 + g2;
			uint32_t b = b1 + b2;
			uint32_t a = a1 + a2;

			RGBA c = { (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a };

			for (uint32_t x = 0; x < m_width; x++)
				*p++ = c;
		}
	}

	void		
	Image::DrawBlendedHorizontalLine(
		uint32_t		aX,
		uint32_t		aY,
		uint32_t		aLength,
		const RGBA&		aColor)
	{
		assert(HasData());
		uint32_t i = aX + aY * m_width;
		assert(i < m_width * m_height);
		assert(i + aLength <= m_width * m_height);
		RGBA* p = m_data + i;

		uint32_t premultR = ((uint32_t)aColor.m_r * (uint32_t)aColor.m_a) / 255;
		uint32_t premultG = ((uint32_t)aColor.m_g * (uint32_t)aColor.m_a) / 255;
		uint32_t premultB = ((uint32_t)aColor.m_b * (uint32_t)aColor.m_a) / 255;

		for(uint32_t x = 0; x < aLength; x++)
		{
			uint32_t r = ((uint32_t)p->m_r * (255 - (uint32_t)aColor.m_a)) / 255;
			uint32_t g = ((uint32_t)p->m_g * (255 - (uint32_t)aColor.m_a)) / 255;
			uint32_t b = ((uint32_t)p->m_b * (255 - (uint32_t)aColor.m_a)) / 255;

			p->m_r = (uint8_t)(r + premultR);
			p->m_g = (uint8_t)(g + premultG);
			p->m_b = (uint8_t)(b + premultB);		
			
			p++;
		}
	}

	void		
	Image::Release()
	{
		if (m_data != NULL)
			free(m_data);

		m_data = NULL;
		m_width = 0;
		m_height = 0;
	}

	void		
	Image::Detach(
		void**			aOutData,
		uint32_t*		aOutWidth,
		uint32_t*		aOutHeight)
	{
		*aOutData = m_data;
		*aOutWidth = m_width;
		*aOutHeight = m_height;

		m_data = NULL;
		m_width = 0;
		m_height = 0;
	}

}