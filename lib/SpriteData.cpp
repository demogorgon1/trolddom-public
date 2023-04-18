#include "Pcheader.h"

#include <kpublic/Compression.h>
#include <kpublic/SpriteData.h>

namespace kpublic
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
			return _ReadBuffer(aPointer, aBytesRemaining, &aOut, sizeof(uint32_t));
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

				sprite.m_rgba = (const RGBA*)p;
				if(!_ReadBuffer(p, bytesRemaining, NULL, sprite.m_width * sprite.m_height * 4))
					return false;

				sheet->m_sprites.push_back(sprite);
			}

			m_sheets.push_back(std::move(sheet));
		}

		if(bytesRemaining > 0)
			return false;

		return true;
	}

}