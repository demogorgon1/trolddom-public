#include "Pcheader.h"

#include <kpublic/Manifest.h>
#include <kpublic/MapData.h>
#include <kpublic/SpriteData.h>

#include "MapImageOutput.h"

namespace kpublic
{

	MapImageOutput::MapImageOutput(
		const char*			aDataPath,
		const Manifest*		aManifest)
		: m_manifest(aManifest)
		, m_dataPath(aDataPath)
	{
		std::string spritesPath = aDataPath;
		spritesPath += "/sprites.bin";

		SpriteData spriteData;
		bool ok = spriteData.Load(spritesPath.c_str());
		KP_CHECK(ok, "Failed to load sprite data.");

		std::vector<std::unique_ptr<Image>> sheets;
		
		for(const std::unique_ptr<SpriteData::Sheet>& sheet : spriteData.m_sheets)
		{
			std::unique_ptr<Image> image = std::make_unique<Image>();
			image->Allocate(sheet->m_width, sheet->m_height);

			for(const SpriteData::Sprite& sprite : sheet->m_sprites)
			{
				Image temp;
				temp.Allocate(sprite.m_width, sprite.m_height);
				memcpy(temp.GetData(), sprite.m_rgba, temp.GetSize());
				image->Insert(sprite.m_sheetX, sprite.m_sheetY, temp);					
			}

			sheets.push_back(std::move(image));
		}

		m_manifest->m_sprites.ForEach([&](
			const Data::Sprite* aSprite)
		{
			KP_CHECK(aSprite->m_spriteSheetIndex < (uint32_t)sheets.size(), "Invalid sprite sheet index.");
			Image* spriteSheet = sheets[aSprite->m_spriteSheetIndex].get();

			std::unique_ptr<Image> image = std::make_unique<Image>();
			image->Allocate(aSprite->m_width, aSprite->m_height);
			spriteSheet->Extract(aSprite->m_offsetX, aSprite->m_offsetY, aSprite->m_width, aSprite->m_height, *image);
			
 			m_tileImageTable[aSprite->m_id] = std::move(image);
		});
	}
	
	MapImageOutput::~MapImageOutput()
	{

	}

	void	
	MapImageOutput::Generate(
		const MapData*		aMap,
		const char*			aPath)
	{
		Image image;
		image.Allocate((uint32_t)aMap->m_width * 16, (uint32_t)aMap->m_height * 16);

		for(int32_t y = 0; y < aMap->m_height; y++)
		{
			for(int32_t x = 0; x < aMap->m_width; x++)
			{
				uint32_t tileSpriteId = aMap->m_tileMap[x + y * aMap->m_width];
				if(tileSpriteId != 0)
				{
					TileImageTable::iterator i = m_tileImageTable.find(tileSpriteId);
					assert(i != m_tileImageTable.end());
					image.Insert((uint32_t)x * 16, (uint32_t)y * 16, *(i->second));
				}
			}
		}

		image.SavePNG(aPath);
	}

}