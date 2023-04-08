#include "Pcheader.h"

#include <kaos-public/Compression.h>

#include "SpriteSheetBuilder.h"

namespace kaos_public
{

	namespace
	{

		uint32_t
		_GetNearestPowerOfTwo(
			uint32_t				aValue)
		{
			uint32_t v = aValue;
			v--;
			v |= v >> 1;
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;
			v++;
			return v;
		}

		void
		_WriteUInt32(
			std::vector<uint8_t>&	aBuffer,
			uint32_t				aValue)
		{
			size_t offset = aBuffer.size();
			aBuffer.resize(offset + sizeof(aValue));
			memcpy(&aBuffer[offset], &aValue, sizeof(aValue));
		}

		void
		_WriteBuffer(
			std::vector<uint8_t>&	aBuffer,
			const void*				aPtr,
			size_t					aSize)
		{
			size_t offset = aBuffer.size();
			aBuffer.resize(offset + aSize);
			memcpy(&aBuffer[offset], aPtr, aSize);
		}

	}

	SpriteSheetBuilder::SpriteSheetBuilder(
		uint32_t					aMinSheetSize)
		: m_minSheetSize(aMinSheetSize)
	{

	}
	
	SpriteSheetBuilder::~SpriteSheetBuilder()
	{

	}

	void	
	SpriteSheetBuilder::AddSprites(
		const Parser::Node*			aSource)
	{
		uint32_t offsetX = 0;
		uint32_t offsetY = 0;
		uint32_t width = 0;
		uint32_t height = 0;

		Image sourceImage;

		aSource->ForEachChild([&](
			const Parser::Node*		aNode)
		{
			if(aNode->m_name == "source")
			{				
				std::string path = aNode->m_path + "/" + aNode->m_value;
				sourceImage.LoadPNG(path.c_str());
			}
			else if(aNode->m_name == "cursor")
			{				
				aNode->GetObject()->ForEachChild([&](
					const Parser::Node* aCursorNode)
				{	
					if(aCursorNode->m_name == "position")
					{
						KP_VERIFY(aCursorNode->m_type == Parser::Node::TYPE_ARRAY && aCursorNode->m_children.size() == 2, aCursorNode->m_debugInfo, "Invalid cursor position.");
						offsetX = aCursorNode->m_children[0]->GetUInt32();
						offsetY = aCursorNode->m_children[1]->GetUInt32();
					}
					else
					{
						KP_VERIFY(false, aCursorNode->m_debugInfo, "Invalid cursor item.");
					}
				});
			}
			else if (aNode->m_name == "size")
			{
				aNode->GetObject()->ForEachChild([&](
					const Parser::Node* aSizeComponent)
				{	
					if(aSizeComponent->m_name == "width")
						width = aSizeComponent->GetUInt32();
					else if (aSizeComponent->m_name == "height")
						height = aSizeComponent->GetUInt32();
					else
						KP_VERIFY(false, aSizeComponent->m_debugInfo, "Invalid size component.");
				});
			}
			else if(aNode->m_tag == "sprite")
			{
				KP_VERIFY(!aNode->m_name.empty(), aNode->m_debugInfo, "Sprite must be named.");
				KP_VERIFY(width > 0 && height > 0, aNode->m_debugInfo, "Sprite must be of non-zero size.");
				KP_VERIFY(sourceImage.HasData(), aNode->m_debugInfo, "No source image specified.");
				KP_VERIFY(offsetX + width <= sourceImage.GetWidth() && offsetY + height <= sourceImage.GetHeight(), aNode->m_debugInfo, "Sprite exceeds source image bounds.");

				Sprite* sprite = _CreateSprite(aNode, aNode->m_name.c_str(), width * height);
				sourceImage.Extract(offsetX, offsetY, width, height, sprite->m_image);		
				
				offsetX += width; // Automatically advance horizontally

				aNode->GetObject()->ForEachChild([&](
					const Parser::Node* aSpriteComponent)
				{
					if(aSpriteComponent->m_name == "flags")
					{
						aSpriteComponent->GetArray()->ForEachChild([&](
							const Parser::Node* aFlag)
						{
							uint8_t flag = SpriteInfo::StringToFlag(aFlag->GetIdentifier());
							KP_VERIFY(flag != 0, aFlag->m_debugInfo, "Not a valid sprite flag.");
							sprite->m_info.m_flags |= flag;
						});
					}
					else
					{
						KP_VERIFY(false, aNode->m_debugInfo, "Invalid item in 'sprite'.");
					}
				});
			}
			else
			{
				KP_VERIFY(false, aNode->m_debugInfo, "Invalid item in 'sprites'.");
			}
		});
	}

	void	
	SpriteSheetBuilder::Build()
	{
		// Sort sprites by size, largest first
		std::vector<Sprite*> sortedSprites;

		{
			for (SpriteTable::iterator i = m_spriteTable.begin(); i != m_spriteTable.end(); i++)
				sortedSprites.push_back(i->second.get());

			std::sort(sortedSprites.begin(), sortedSprites.end(), [&](
				Sprite* aLHS,
				Sprite* aRHS)
			{
				return aLHS->m_size > aRHS->m_size;
			});
		}

		// Insert sprites in sheets, largest first
		for(Sprite* sprite : sortedSprites)
			_InsertSprite(sprite);
	}

	void	
	SpriteSheetBuilder::ExportManifestData(
		PersistentIdTable*		aPersistentIdTable,
		Manifest*				aManifest)
	{
		for (SpriteTable::iterator i = m_spriteTable.begin(); i != m_spriteTable.end(); i++)
		{
			Sprite* sprite = i->second.get();
			Data::Sprite* data = aManifest->m_sprites.GetByName(aPersistentIdTable, sprite->m_name.c_str());

			data->m_offsetX = sprite->m_sheetOffsetX;
			data->m_offsetY = sprite->m_sheetOffsetY;
			data->m_width = sprite->m_image.GetWidth();
			data->m_height = sprite->m_image.GetHeight();
			data->m_spriteSheetIndex = sprite->m_sheetIndex;
			data->m_info = sprite->m_info;
			data->m_defined = true;
		}
	}

	void	
	SpriteSheetBuilder::ExportSheets(
		const char*				aPath)
	{
		// Serialize to big data blob
		std::vector<uint8_t> compressed;

		{
			std::vector<uint8_t> data;
			_WriteUInt32(data, (uint32_t)m_sheets.size());

			for (std::unique_ptr<Sheet>& sheet : m_sheets)
			{
				_WriteUInt32(data, sheet->m_width);
				_WriteUInt32(data, sheet->m_height);

				_WriteUInt32(data, (uint32_t)sheet->m_sprites.size());

				for(const Sprite* sprite : sheet->m_sprites)
				{
					_WriteUInt32(data, sprite->m_sheetOffsetX);
					_WriteUInt32(data, sprite->m_sheetOffsetY);
					_WriteUInt32(data, sprite->m_image.GetWidth());
					_WriteUInt32(data, sprite->m_image.GetHeight());
					_WriteBuffer(data, sprite->m_image.GetData(), sprite->m_image.GetSize());
				}
			}

			Compression::Pack(&data[0], data.size(), compressed);
		}

		// Write to file
		FILE* f = fopen(aPath, "wb");
		KP_CHECK(f != NULL, "Failed to open file for output: %s", aPath);
		fwrite(&compressed[0], 1, compressed.size(), f);
		fclose(f);
	}

	//---------------------------------------------------------------------------

	SpriteSheetBuilder::Sprite* 
	SpriteSheetBuilder::_CreateSprite(
		const Parser::Node*		aNode,
		const char*				aName,
		uint32_t				aSize)
	{
		KP_VERIFY(m_spriteTable.find(aName) == m_spriteTable.end(), aNode->m_debugInfo, "Sprite name '%s' already in use.", aName);
		Sprite* sprite = new Sprite(aName, aSize);
		m_spriteTable[aName] = std::unique_ptr<Sprite>(sprite);
		return sprite;
	}

	SpriteSheetBuilder::Sheet* 
	SpriteSheetBuilder::_CreateSheet(
		uint32_t				aWidth,
		uint32_t				aHeight)
	{
		Sheet* sheet = new Sheet((uint32_t)m_sheets.size(), aWidth, aHeight);
		m_sheets.push_back(std::unique_ptr<Sheet>(sheet));
		return sheet;
	}

	void	
	SpriteSheetBuilder::_InsertSprite(
		Sprite*					aSprite)
	{
		// FIXME: this is brute force, not optimal, and could be optimized
		uint32_t width = aSprite->m_image.GetWidth();
		uint32_t height = aSprite->m_image.GetHeight();

		Sheet* insertSheet = NULL;
		size_t emptySpaceIndex = 0;
		uint32_t emptySpaceSize = 0;

		// Find smallest empty space that fits this sprite
		{
			for (std::unique_ptr<Sheet>& sheet : m_sheets)
			{
				for(size_t i = 0; i < sheet->m_emptySpaces.size(); i++)
				{
					Sheet::EmptySpace& emptySpace = sheet->m_emptySpaces[i];

					if(width <= emptySpace.m_width && height <= emptySpace.m_height)
					{
						uint32_t size = emptySpace.m_width * emptySpace.m_height;
						if(insertSheet == NULL || size < emptySpaceSize)
						{
							insertSheet = sheet.get();
							emptySpaceIndex = i;
							emptySpaceSize = size;
						}
					}
				}
			}
		}

		// If no empty space was found, create new blank sheet
		if(insertSheet == NULL)
		{
			uint32_t nearestPowerOfTwoWidth = _GetNearestPowerOfTwo(width);
			uint32_t nearestPowerOfTwoHeight = _GetNearestPowerOfTwo(width);
			uint32_t sheetSize = std::max(nearestPowerOfTwoWidth, nearestPowerOfTwoHeight);
			sheetSize = std::max(sheetSize, m_minSheetSize);
			insertSheet = _CreateSheet(sheetSize, sheetSize);
		}

		// Insert
		{
			assert(emptySpaceIndex < insertSheet->m_emptySpaces.size());
			Sheet::EmptySpace& emptySpace = insertSheet->m_emptySpaces[emptySpaceIndex];

			aSprite->m_sheetIndex = insertSheet->m_index;
			aSprite->m_sheetOffsetX = emptySpace.m_x;
			aSprite->m_sheetOffsetY = emptySpace.m_y;

			if(emptySpace.m_width == width && emptySpace.m_height > height)
			{
				/*
				    XXXX
					XXXX
					aaaa <-- adjusted empty space
					aaaa
				*/
				emptySpace.m_y += height;
				emptySpace.m_height -= height;
			}
			else if (emptySpace.m_width > width && emptySpace.m_height == height)
			{
				/*
					XXaa <-- adjusted empty space
					XXaa
					XXaa
					XXaa
				*/
				emptySpace.m_x += width;
				emptySpace.m_width -= width;
			}
			else if (emptySpace.m_width > width && emptySpace.m_height > height)
			{
				/*
					XXaa <-- adjusted empty space
					XXaa
					bbbb <-- inserted empty space
					bbbb
				*/
				Sheet::EmptySpace insertSpace =
				{
					emptySpace.m_x, emptySpace.m_y + height,
					emptySpace.m_width, emptySpace.m_height - height 
				};

				emptySpace.m_x += width;
				emptySpace.m_width -= width;
				emptySpace.m_height = height;

				insertSheet->m_emptySpaces.push_back(insertSpace);
			}
			else 
			{
				// Must be perfect match
				assert(emptySpace.m_width == width && emptySpace.m_height == height);
				insertSheet->m_emptySpaces.erase(insertSheet->m_emptySpaces.begin() + emptySpaceIndex);
			}

			insertSheet->m_sprites.push_back(aSprite);
		}
	}


}