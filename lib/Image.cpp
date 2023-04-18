#include "Pcheader.h"

#include <lodepng/lodepng.h>

#include "Image.h"

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
	Image::Release()
	{
		if (m_data != NULL)
			free(m_data);

		m_data = NULL;
		m_width = 0;
		m_height = 0;
	}

}