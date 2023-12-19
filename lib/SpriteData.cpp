#include "Pcheader.h"

#include <tpublic/Compression.h>
#include <tpublic/SpriteData.h>

namespace tpublic
{

	namespace
	{

		bool
		_LoadFile(
			const char*				aPath,
			std::vector<uint8_t>&	aOut)
		{
			FILE* f = fopen(aPath, "rb");
			if(f == NULL)
				return false;
			
			fseek(f, 0, SEEK_END);
			aOut.resize((size_t)ftell(f));
			fseek(f, 0, SEEK_SET);

			size_t bytes = fread(&aOut[0], 1, aOut.size(), f);
			if(bytes != aOut.size())
			{
				fclose(f);
				return false;
			}

			fclose(f);

			return true;
		}

		bool
		_ReadBuffer(
			const uint8_t*&			aPointer,
			size_t&					aBytesRemaining,
			void*					aBuffer,
			size_t					aBufferSize)
		{
			if(aBytesRemaining < aBufferSize)			
				return false;

			if(aBuffer != NULL)
				memcpy(aBuffer, aPointer, aBufferSize);

			aPointer += aBufferSize;
			aBytesRemaining -= aBufferSize;
			return true;
		}

		bool
		_ReadUInt32(
			const uint8_t*&			aPointer,
			size_t&					aBytesRemaining,
			uint32_t&				aOut)
		{
			uint64_t value = 0;
			uint64_t offset = 0;

			for (;;)
			{
				if(aBytesRemaining == 0)
					return false;

				uint64_t i = (uint64_t)*aPointer;
				aPointer++;
				aBytesRemaining--;

				value |= (i & 0x7FULL) << offset;
				offset += 7;

				if ((i & 0x80ULL) == 0)
					break;
			}

			if(value > (uint64_t)UINT32_MAX)
				return false;

			aOut = (uint32_t)value;
			return true;
		}

	}

	//------------------------------------------------------------------

	SpriteData::SpriteData()
	{

	}
	
	SpriteData::~SpriteData()
	{

	}

	bool		
	SpriteData::Load(
		const char* aPath)
	{
		std::vector<uint8_t> compressed;
		if(!_LoadFile(aPath, compressed))
			return false;

		if(!Compression::Unpack(&compressed[0], compressed.size(), m_data))
			return false;

		size_t bytesRemaining = m_data.size();
		const uint8_t* p = &m_data[0];

		uint32_t totalPixelCount = 0;
		if (!_ReadUInt32(p, bytesRemaining, totalPixelCount))
			return false;

		m_rgbaData.resize((size_t)totalPixelCount);
		size_t rgbaDataOffset = 0;

		uint32_t paletteSize = 0;
		if (!_ReadUInt32(p, bytesRemaining, paletteSize))
			return false;

		std::vector<uint32_t> palette;
		palette.resize((size_t)paletteSize);
		if(!_ReadBuffer(p, bytesRemaining, &palette[0], palette.size() * sizeof(uint32_t)))
			return false;

		uint32_t sheetCount = 0;
		if(!_ReadUInt32(p, bytesRemaining, sheetCount))
			return false;

		for(uint32_t i = 0; i < sheetCount; i++)
		{
			std::unique_ptr<Sheet> sheet = std::make_unique<Sheet>();

			if (!_ReadUInt32(p, bytesRemaining, sheet->m_width))
				return false;
			if (!_ReadUInt32(p, bytesRemaining, sheet->m_height))
				return false;

			if(sheet->m_width == 0 || sheet->m_width > 2048)
				return false;
			if (sheet->m_height == 0 || sheet->m_height > 2048)
				return false;

			uint32_t spriteCount;
			if (!_ReadUInt32(p, bytesRemaining, spriteCount))
				return false;

			for(uint32_t j = 0; j < spriteCount; j++)
			{
				Sprite sprite;
				sprite.m_sheetIndex = i;

				if (!_ReadUInt32(p, bytesRemaining, sprite.m_sheetX))
					return false;
				if (!_ReadUInt32(p, bytesRemaining, sprite.m_sheetY))
					return false;
				if (!_ReadUInt32(p, bytesRemaining, sprite.m_width))
					return false;
				if (!_ReadUInt32(p, bytesRemaining, sprite.m_height))
					return false;

				if (sprite.m_width == 0 || sprite.m_width > 2048)
					return false;
				if (sprite.m_height == 0 || sprite.m_height > 2048)
					return false;
				
				if (sprite.m_sheetX + sprite.m_width > sheet->m_width)
					return false;
				if (sprite.m_sheetY + sprite.m_height > sheet->m_height)
					return false;

				if(rgbaDataOffset + (size_t)(sprite.m_width * sprite.m_height) > m_rgbaData.size())
					return false;

				sprite.m_rgba = (const RGBA*)&m_rgbaData[rgbaDataOffset];

				for(uint32_t k = 0, count = sprite.m_width * sprite.m_height; k < count; k++)
				{
					uint32_t paletteIndex = 0;
					if (!_ReadUInt32(p, bytesRemaining, paletteIndex))
						return false;

					if(paletteIndex >= paletteSize)
						return false;

					uint32_t paletteRGBA = palette[paletteIndex];
					assert((size_t)rgbaDataOffset < m_rgbaData.size());
					RGBA& rgba = m_rgbaData[rgbaDataOffset++];
					memcpy(&rgba, &paletteRGBA, sizeof(uint32_t));
				}

				sheet->m_sprites.push_back(sprite);
			}

			m_sheets.push_back(std::move(sheet));
		}

		if(bytesRemaining > 0)
			return false;

		return true;
	}

}