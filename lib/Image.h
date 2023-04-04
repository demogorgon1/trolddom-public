#pragma once

#include <kaos-public/DataErrorHandling.h>

namespace kaos_public
{

	class Image
	{
	public:
		struct RGBA
		{
			uint8_t		m_r;
			uint8_t		m_g;
			uint8_t		m_b;
			uint8_t		m_a;
		};

					Image();
					Image(
						uint32_t		aWidth,
						uint32_t		aHeight);
					~Image();

		void		Allocate(
						uint32_t		aWidth,
						uint32_t		aHeight);
		void		LoadPNG(
						const char*		aPath);
		void		SavePNG(
						const char*		aPath) const;
		void		Extract(
						uint32_t		aX,
						uint32_t		aY,
						uint32_t		aWidth,
						uint32_t		aHeight,
						Image&			aOut) const;
		void		Insert(
						uint32_t		aX,
						uint32_t		aY,
						const Image&	aImage);
		void		Clear(
						const RGBA&		aColor);
		void		Release();
		
		// Data access
		uint32_t	GetWidth() const { return m_width; }
		uint32_t	GetHeight() const { return m_height; }
		bool		HasData() const { return m_data != NULL; }
		const RGBA*	GetData() const { assert(m_data != NULL); return m_data; }
		RGBA*		GetData() { assert(m_data != NULL); return m_data; }
		size_t		GetSize() const { return (size_t)m_width * (size_t)m_height * sizeof(RGBA); }

	private:

		uint32_t	m_width;
		uint32_t	m_height;
		RGBA*		m_data;
	};

}