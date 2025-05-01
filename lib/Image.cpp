#include "Pcheader.h"

#include <lodepng/lodepng.h>

#include <tpublic/Image.h>

namespace tpublic
{

	namespace
	{

		struct SmallPixelFont
		{
			static const uint32_t XSIZE = 6;
			static const uint32_t YSIZE = 7;

			SmallPixelFont()
			{
				memset(m_characterOffsets, 0xFF, sizeof(m_characterOffsets));
				m_numCharacters = (uint32_t)strlen(m_characters);
				for(uint32_t i = 0; i < m_numCharacters; i++)
				{
					uint8_t character = (uint8_t)m_characters[i];
					m_characterOffsets[character] = (uint8_t)i;
				}
				assert(strlen(m_data) == (size_t)(XSIZE * YSIZE * m_numCharacters));
			}

			bool
			GetCharacterPixel(
				char		aCharacter,
				uint32_t	aX,
				uint32_t	aY) const
			{
				assert(aX < XSIZE && aY < YSIZE);
				uint32_t offset = m_characterOffsets[(uint8_t)aCharacter];
				return offset != 0xFF && m_data[aX + offset * XSIZE + aY * (XSIZE * m_numCharacters)] == 'x';
			}

			// Public data
			uint8_t		m_characterOffsets[256];
			uint32_t	m_numCharacters = 0;
			const char* m_characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,._-";
			const char*	m_data = 
				".xxx..xxxx...xxxx.xxxx..xxxxx.xxxxx..xxx..x...x.xxxxx.xxxxx.x...x.x.....x...x.x...x..xxx..xxxx...xxx..xxxx...xxxx.xxxxx.x...x.x...x.x...x.x...x.x...x.xxxxx..xxx....x....xxx...xxx..x...x.xxxxx..xxx..xxxxx..xxx...xxx.........................."
				"x...x.x...x.x.....x...x.x.....x.....x...x.x...x...x.......x.x..x..x.....xx.xx.xx..x.x...x.x...x.x...x.x...x.x.......x...x...x.x...x.x...x.x...x.x...x.....x.x...x..xx...x...x.x...x.x...x.x.....x...x.....x.x...x.x...x........................."
				"x...x.x...x.x.....x...x.x.....x.....x.....x...x...x.......x.x.x...x.....x.x.x.x.x.x.x...x.x...x.x...x.x...x.x.......x...x...x.x...x.x...x..x.x..x...x....x..x...x.x.x......x......x.x...x.x.....x.........x.x...x.x...x........................."
				"xxxxx.xxxx..x.....x...x.xxx...xxx...x.xxx.xxxxx...x.......x.xx....x.....x...x.x..xx.x...x.xxxx..x...x.xxxx...xxx....x...x...x.x...x.x...x...x....x.x....x...x.x.x...x.....x......x..xxxxx.xxxx..xxxx.....x...xxx...xxxx....................xxx.."
				"x...x.x...x.x.....x...x.x.....x.....x...x.x...x...x.......x.x.x...x.....x...x.x...x.x...x.x.....x.x.x.x.x.......x...x...x...x.x...x.x.x.x..x.x....x....x....x...x...x....x........x.....x.....x.x...x...x...x...x.....x........................."
				"x...x.x...x.x.....x...x.x.....x.....x...x.x...x...x.......x.x..x..x.....x...x.x...x.x...x.x.....x..xx.x..x......x...x...x...x..x.x..xx.xx.x...x...x...x.....x...x...x...x.....x...x.....x.x...x.x...x...x...x...x.x...x...x....................."
				"x...x.xxxx...xxxx.xxxx..xxxxx.x......xxx..x...x.xxxxx.xxxx..x...x.xxxxx.x...x.x...x..xxx..x......xxxx.x...x.xxxx....x....xxx....x...x...x.x...x...x...xxxxx..xxx..xxxxx.xxxxx..xxx......x..xxx...xxx....x....xxx...xxx...x.....x....xxxxx.......";
		};

		SmallPixelFont g_smallPixelFont;
	}

	//---------------------------------------------------------------------------------

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
	Image::Copy(
		const Image&	aImage)
	{
		assert(aImage.HasData());
		
		Allocate(aImage.GetWidth(), aImage.GetHeight());
		memcpy(m_data, aImage.GetData(), aImage.GetSize());
	}

	void		
	Image::LoadPNG(
		const char*		aPath)
	{
		std::filesystem::path p = aPath;
		TP_CHECK(p.extension() == ".png", "Not a PNG: %s", aPath);

		Release();
		
		uint32_t result = (uint32_t)lodepng_decode32_file((unsigned char**)&m_data, (unsigned*)&m_width, (unsigned*)&m_height, aPath);
		TP_CHECK(result == 0, "Failed to load PNG: %s", aPath);
	}
	
	void		
	Image::SavePNG(
		const char*		aPath) const
	{
		std::filesystem::path p = aPath;
		TP_CHECK(p.extension() == ".png", "Not a PNG: %s", aPath);

		assert(HasData());
		assert(m_width > 0 && m_height > 0);

		uint32_t result = (uint32_t)lodepng_encode32_file(aPath, (const unsigned char*)m_data, (unsigned)m_width, (unsigned)m_height);
		TP_CHECK(result == 0, "Failed to save PNG: %s", aPath);
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
	Image::InsertBlended(
		uint32_t		aX,
		uint32_t		aY,
		const Image&	aImage)
	{
		assert(HasData());
		assert(aImage.HasData());
		assert(aX + aImage.m_width <= m_width && aY + aImage.m_height <= m_height);

		for(uint32_t y = 0; y < aImage.m_height; y++)
		{
			RGBA* out = GetData() + aX + (aY + y) * m_width;
			const RGBA* in = aImage.GetData() + y * aImage.GetWidth();

			for(uint32_t x = 0; x < aImage.m_width; x++)
			{
				uint32_t r1 = ((uint32_t)out->m_r * (uint32_t)(255 - in->m_a)) / 255;
				uint32_t g1 = ((uint32_t)out->m_g * (uint32_t)(255 - in->m_a)) / 255;
				uint32_t b1 = ((uint32_t)out->m_b * (uint32_t)(255 - in->m_a)) / 255;
				uint32_t a1 = ((uint32_t)out->m_a * (uint32_t)(255 - in->m_a)) / 255;

				uint32_t r2 = ((uint32_t)in->m_r * (uint32_t)in->m_a) / 255;
				uint32_t g2 = ((uint32_t)in->m_g * (uint32_t)in->m_a) / 255;
				uint32_t b2 = ((uint32_t)in->m_b * (uint32_t)in->m_a) / 255;
				uint32_t a2 = ((uint32_t)in->m_a * (uint32_t)in->m_a) / 255;

				uint32_t r = r1 + r2;
				uint32_t g = g1 + g2;
				uint32_t b = b1 + b2;
				uint32_t a = a1 + a2;

				RGBA c = { (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a };

				*out = c;

				out++;
				in++;
			}
		}
	}
	
	void		
	Image::InsertRaw(
		uint32_t					aX,
		uint32_t					aY,
		const void*					aData,
		uint32_t					aWidth,
		uint32_t					aHeight,
		uint32_t					aPitch)
	{
		assert(HasData());
		assert(aX + aWidth <= m_width && aY + aHeight <= m_height);

		for (uint32_t y = 0; y < aHeight; y++)
			memcpy(GetData() + aX + (aY + y) * m_width, (const uint8_t*)aData + y * aPitch, sizeof(RGBA) * aWidth);
	}

	void		
	Image::InsertRawBlended(
		uint32_t					aX,
		uint32_t					aY,
		const void*					aData,
		uint32_t					aWidth,
		uint32_t					aHeight,
		uint32_t					aPitch)
	{
		assert(HasData());
		assert(aX + aWidth <= m_width && aY + aHeight <= m_height);

		for (uint32_t y = 0; y < aHeight; y++)
		{
			RGBA* out = GetData() + aX + (aY + y) * m_width;
			const RGBA* in = (const RGBA*)((const uint8_t*)aData + y * aPitch);

			for (uint32_t x = 0; x < aWidth; x++)
			{
				uint32_t r1 = ((uint32_t)out->m_r * (uint32_t)(255 - in->m_a)) / 255;
				uint32_t g1 = ((uint32_t)out->m_g * (uint32_t)(255 - in->m_a)) / 255;
				uint32_t b1 = ((uint32_t)out->m_b * (uint32_t)(255 - in->m_a)) / 255;
				uint32_t a1 = ((uint32_t)out->m_a * (uint32_t)(255 - in->m_a)) / 255;

				uint32_t r2 = ((uint32_t)in->m_r * (uint32_t)in->m_a) / 255;
				uint32_t g2 = ((uint32_t)in->m_g * (uint32_t)in->m_a) / 255;
				uint32_t b2 = ((uint32_t)in->m_b * (uint32_t)in->m_a) / 255;
				uint32_t a2 = ((uint32_t)in->m_a * (uint32_t)in->m_a) / 255;

				uint32_t r = r1 + r2;
				uint32_t g = g1 + g2;
				uint32_t b = b1 + b2;
				uint32_t a = a1 + a2;

				RGBA c = { (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a };

				*out = c;

				out++;
				in++;
			}
		}
	}

	void		
	Image::InsertRawBlendedWithColor(
		uint32_t					aX,
		uint32_t					aY,
		const void*					aData,
		uint32_t					aWidth,
		uint32_t					aHeight,
		uint32_t					aPitch,
		const RGBA&					aColor)
	{
		assert(HasData());
		assert(aX + aWidth <= m_width && aY + aHeight <= m_height);

		for (uint32_t y = 0; y < aHeight; y++)
		{
			RGBA* out = GetData() + aX + (aY + y) * m_width;
			const RGBA* in = (const RGBA*)((const uint8_t*)aData + y * aPitch);

			for (uint32_t x = 0; x < aWidth; x++)
			{
				uint8_t inA = (uint8_t)(((uint32_t)in->m_a * (uint32_t)aColor.m_a) / 255);

				uint32_t r1 = ((uint32_t)out->m_r * (uint32_t)(255 - inA)) / 255;
				uint32_t g1 = ((uint32_t)out->m_g * (uint32_t)(255 - inA)) / 255;
				uint32_t b1 = ((uint32_t)out->m_b * (uint32_t)(255 - inA)) / 255;

				uint32_t r2 = ((((uint32_t)in->m_r * (uint32_t)inA) / 255) * (uint32_t)aColor.m_r) / 255;
				uint32_t g2 = ((((uint32_t)in->m_g * (uint32_t)inA) / 255) * (uint32_t)aColor.m_g) / 255;
				uint32_t b2 = ((((uint32_t)in->m_b * (uint32_t)inA) / 255) * (uint32_t)aColor.m_b) / 255;

				uint32_t r = r1 + r2;
				uint32_t g = g1 + g2;
				uint32_t b = b1 + b2;
				uint32_t a = 255;

				RGBA c = { (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a };

				*out = c;

				out++;
				in++;
			}
		}
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
	Image::Grow(
		uint32_t					aFactor,
		const std::optional<RGBA>&	aGridColor,
		const std::optional<Vec2>&	aGridOffset)
	{
		assert(HasData());
		assert(aFactor > 1);

		Image grown(m_width * aFactor, m_height * aFactor);
		RGBA* out = grown.GetData();
		const RGBA* in = GetData();
		for(uint32_t y = 0; y < grown.m_height; y++)
		{
			for (uint32_t x = 0; x < grown.m_width; x++)
			{
				uint32_t inX = x / aFactor;
				uint32_t inY = y / aFactor;
				RGBA inColor = in[inX + inY * m_width];;

				if(aGridColor.has_value() && (x % aFactor == 0 || y % aFactor == 0))
					inColor = aGridColor.value();

				*out = inColor;
				out++;
			}
		}

		if(aGridColor.has_value())
		{
			for(uint32_t y = 0; y < m_height; y++)
			{
				for (uint32_t x = 0; x < m_width; x++)
				{
					int32_t dx = (int32_t)x;
					int32_t dy = (int32_t)y;
					if(aGridOffset.has_value())
					{
						dx += aGridOffset->m_x;
						dy += aGridOffset->m_y;
					}
					
					grown.DrawString(x * aFactor + 2, y * aFactor + 2, "%d,%d", dx, dy);
				}
			}
		}

		Release();
		grown.Detach((void**)&m_data, &m_width, &m_height);
	}

	void		
	Image::DrawString(
		uint32_t					aX,
		uint32_t					aY,
		const char*					aFormat,
		...)
	{
		assert(HasData());

		char buffer[1024];
		TP_STRING_FORMAT_VARARGS(buffer, sizeof(buffer), aFormat);

		const char* p = buffer;
		uint32_t xPosition = aX;
		uint32_t yPosition = aY;
		RGBA* out = GetData();

		while(*p != '\0')
		{			
			for(uint32_t y = 0; y < SmallPixelFont::YSIZE; y++)
			{
				for (uint32_t x = 0; x < SmallPixelFont::XSIZE; x++)
				{
					uint32_t xImage = xPosition + x;
					uint32_t yImage = yPosition + y;

					if(xImage < m_width && yImage < m_height)
					{
						if(g_smallPixelFont.GetCharacterPixel(*p, x, y))
						{	
							RGBA& c = out[xImage + yImage * m_width];
							
							c.m_r = 255 - c.m_r;
							c.m_g = 255 - c.m_g;
							c.m_b = 255 - c.m_b;

							if(c.m_r > 64)
								c.m_r -= 64;
							else
								c.m_r = 0;

							if (c.m_g > 64)
								c.m_g -= 64;
							else
								c.m_g = 0;

							if (c.m_b > 64)
								c.m_b -= 64;
							else
								c.m_b = 0;

							c.m_a = 255;
						}
					}
				}
			}
			p++;
			xPosition += SmallPixelFont::XSIZE;

			if(xPosition + SmallPixelFont::XSIZE > m_width)
			{
				xPosition = aX;
				yPosition += SmallPixelFont::YSIZE;
			}
		}
	}

	void		
	Image::MakeGreyscale(
		uint32_t			aAlpha)
	{
		assert(HasData());

		RGBA* p = m_data;
		for(uint32_t i = 0, count = m_width * m_height; i < count; i++)
		{
			uint32_t c = ((uint32_t)p->m_r + (uint32_t)p->m_g + (uint32_t)p->m_b) / 3;
			p->m_r = (uint8_t)((c * aAlpha + (uint32_t)p->m_r * (255 - aAlpha)) / 255);
			p->m_g = (uint8_t)((c * aAlpha + (uint32_t)p->m_g * (255 - aAlpha)) / 255);
			p->m_b = (uint8_t)((c * aAlpha + (uint32_t)p->m_b * (255 - aAlpha)) / 255);
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

	Image::RGBA		
	Image::GetAverageColor() const
	{
		if (m_data == NULL || m_width == 0 || m_height == 0)
			return RGBA();
			
		uint64_t r = 0;
		uint64_t g = 0;
		uint64_t b = 0;
		uint64_t a = 0;

		uint32_t count = m_width * m_height;
		const RGBA* p = m_data;

		for (uint32_t i = 0; i < count; i++)
		{		
			r += (uint64_t)p->m_r;
			g += (uint64_t)p->m_g;
			b += (uint64_t)p->m_b;
			a += (uint64_t)p->m_a;
			p++;
		}

		return RGBA((uint8_t)(r / (uint64_t)count), (uint8_t)(g / (uint64_t)count), (uint8_t)(b / (uint64_t)count), (uint8_t)(a / (uint64_t)count));
	}

}
