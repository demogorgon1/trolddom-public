#pragma once

#include "DataErrorHandling.h"
#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"
#include "Vec2.h"

namespace tpublic
{

	class Image
	{
	public:
		struct RGBA
		{
			RGBA(
				uint8_t								aR = 0,
				uint8_t								aG = 0,
				uint8_t								aB = 0,
				uint8_t								aA = 0)
				: m_r(aR)
				, m_g(aG)
				, m_b(aB)
				, m_a(aA)
			{

			}

			RGBA(
				const SourceNode*					aSource)
			{
				const SourceNode* p = aSource->GetArray();
				size_t count = p->m_children.size();
				TP_VERIFY(count == 3 || count == 4, aSource->m_debugInfo, "Not a valid RGBA color.");
				m_r = p->m_children[0]->GetUInt8();
				m_g = p->m_children[1]->GetUInt8();
				m_b = p->m_children[2]->GetUInt8();
				m_a = count == 4 ? p->m_children[3]->GetUInt8() : 255;
			}

			RGBA(
				std::mt19937&						aRandom)
				: m_r((uint8_t)(aRandom() % 0xFF))
				, m_g((uint8_t)(aRandom() % 0xFF))
				, m_b((uint8_t)(aRandom() % 0xFF))
				, m_a(255)
			{
			}

			// Public data
			uint8_t		m_r;
			uint8_t		m_g;
			uint8_t		m_b;
			uint8_t		m_a;
		};

					Image();
					Image(
						uint32_t					aWidth,
						uint32_t					aHeight);
					~Image();

		void		Allocate(
						uint32_t					aWidth,
						uint32_t					aHeight);
		void		Copy(
						const Image&				aImage);
		void		LoadPNG(
						const char*					aPath);
		void		SavePNG(
						const char*					aPath) const;
		void		Extract(
						uint32_t					aX,
						uint32_t					aY,
						uint32_t					aWidth,
						uint32_t					aHeight,
						Image&						aOut) const;
		void		Insert(
						uint32_t					aX,
						uint32_t					aY,
						const Image&				aImage);
		void		InsertBlended(
						uint32_t					aX,
						uint32_t					aY,
						const Image&				aImage);
		void		InsertRaw(
						uint32_t					aX,
						uint32_t					aY,
						const void*					aData,
						uint32_t					aWidth,
						uint32_t					aHeight,
						uint32_t					aPitch);
		void		Clear(
						const RGBA&					aColor);
		void		DrawVerticalGradient(
						const RGBA&					aColorTop,
						const RGBA&					aColorBottom);
		void		DrawBlendedHorizontalLine(
						uint32_t					aX,
						uint32_t					aY,
						uint32_t					aLength,
						const RGBA&					aColor);
		void		Grow(
						uint32_t					aFactor,
						const std::optional<RGBA>&	aGridColor = std::optional<RGBA>(),
						const std::optional<Vec2>&	aGridOffset = std::optional<Vec2>());
		void		DrawString(
						uint32_t					aX,
						uint32_t					aY,
						const char*					aFormat,
						...);
		void		MakeGreyscale(
						uint32_t					aAlpha);
		void		Release();
		void		Detach(
						void**						aOutData,
						uint32_t*					aOutWidth,
						uint32_t*					aOutHeight);
		RGBA		GetAverageColor() const;
		
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