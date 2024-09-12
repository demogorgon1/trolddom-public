#include "Pcheader.h"

#include <map>

#include <tpublic/Compression.h>

#include "SpriteSheetBuilder.h"

namespace tpublic
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
		_WriteBuffer(
			std::vector<uint8_t>&	aBuffer,
			const void*				aPtr,
			size_t					aSize)
		{
			size_t offset = aBuffer.size();
			aBuffer.resize(offset + aSize);
			memcpy(&aBuffer[offset], aPtr, aSize);
		}

		void
		_WriteUInt32(
			std::vector<uint8_t>&	aBuffer,
			uint32_t				aValue)
		{
			uint8_t temp[5];
			uint8_t* p = temp;
			size_t i = 0;

			for (;;)
			{
				assert(i < sizeof(temp));

				if (aValue <= 0x7F)
				{
					p[i++] = (uint8_t)aValue;
					break;
				}
				else
				{
					p[i++] = (uint8_t)((aValue & 0x7F) | 0x80);
					aValue >>= 7;
				}
			}

			_WriteBuffer(aBuffer, temp, i);
		}

	}

	SpriteSheetBuilder::SpriteSheetBuilder(
		uint32_t					aMinSheetSize)
		: m_minSheetSize(aMinSheetSize)
		, m_nextUnnamedIndex(0)
	{

	}
	
	SpriteSheetBuilder::~SpriteSheetBuilder()
	{

	}

	void	
	SpriteSheetBuilder::AddSprites(
		const SourceNode*			aSource)
	{
		uint32_t offsetX = 0;
		uint32_t offsetY = 0;
		uint32_t width = 0;
		uint32_t height = 0;

		Image sourceImage;

		bool makeAltGreyscale = false;

		aSource->ForEachChild([&](
			const SourceNode*		aNode)
		{
			if(aNode->m_name == "source")
			{				
				std::string path = aNode->m_realPath + "/" + aNode->m_value;
				sourceImage.LoadPNG(path.c_str());
			}
			else if(aNode->m_name == "make_alt_greyscale")
			{
				makeAltGreyscale = aNode->GetBool();
			}
			else if(aNode->m_name == "cursor")
			{				
				aNode->GetObject()->ForEachChild([&](
					const SourceNode* aCursorNode)
				{	
					if(aCursorNode->m_name == "position")
					{
						TP_VERIFY(aCursorNode->m_type == SourceNode::TYPE_ARRAY && aCursorNode->m_children.size() == 2, aCursorNode->m_debugInfo, "Invalid cursor position.");
						offsetX = aCursorNode->m_children[0]->GetUInt32();
						offsetY = aCursorNode->m_children[1]->GetUInt32();
					}
					else
					{
						TP_VERIFY(false, aCursorNode->m_debugInfo, "Invalid cursor item.");
					}
				});
			}
			else if (aNode->m_name == "size")
			{
				if(aNode->m_type == SourceNode::TYPE_IDENTIFIER && aNode->m_value == "source")
				{
					width = sourceImage.GetWidth();
					height = sourceImage.GetHeight();
				}
				else
				{
					aNode->GetObject()->ForEachChild([&](
						const SourceNode* aSizeComponent)
					{	
						if(aSizeComponent->m_name == "width")
							width = aSizeComponent->GetUInt32();
						else if (aSizeComponent->m_name == "height")
							height = aSizeComponent->GetUInt32();
						else
							TP_VERIFY(false, aSizeComponent->m_debugInfo, "Invalid size component.");
					});
				}
			}
			else if(aNode->m_name == "skip")
			{
				offsetX += width; 
			}
			else if(aNode->m_tag == "sprite")
			{
				TP_VERIFY(!aNode->m_name.empty(), aNode->m_debugInfo, "Sprite must be named.");
				TP_VERIFY(width > 0 && height > 0, aNode->m_debugInfo, "Sprite must be of non-zero size.");
				TP_VERIFY(sourceImage.HasData(), aNode->m_debugInfo, "No source image specified.");
				TP_VERIFY(offsetX + width <= sourceImage.GetWidth() && offsetY + height <= sourceImage.GetHeight(), aNode->m_debugInfo, "Sprite exceeds source image bounds.");

				Sprite* sprite = _CreateSprite(aNode, aNode->m_name.c_str(), width * height);
				sourceImage.Extract(offsetX, offsetY, width, height, sprite->m_image);		

				sprite->m_info.m_averageColor = sprite->m_image.GetAverageColor();

				offsetX += width; // Automatically advance horizontally

				aNode->GetObject()->ForEachChild([&](
					const SourceNode* aSpriteComponent)
				{
					if(aSpriteComponent->m_name == "flags")
					{
						aSpriteComponent->GetArray()->ForEachChild([&](
							const SourceNode* aFlag)
						{
							uint8_t flag = SpriteInfo::StringToFlag(aFlag->GetIdentifier());
							TP_VERIFY(flag != 0, aFlag->m_debugInfo, "Not a valid sprite flag.");
							sprite->m_info.m_flags |= flag;
						});
					}
					else if(aSpriteComponent->m_name == "tile_layer")
					{
						sprite->m_info.m_tileLayer = aSpriteComponent->GetUInt32();
					}
					else if (aSpriteComponent->m_name == "dead")
					{
						sprite->m_info.m_deadSpriteId = aSpriteComponent->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aSpriteComponent->GetIdentifier());
					}
					else if (aSpriteComponent->m_name == "water_floor")
					{
						sprite->m_info.m_waterFloorSpriteId = aSpriteComponent->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aSpriteComponent->GetIdentifier());
					}
					else if (aSpriteComponent->m_name == "tags")
					{
						aSpriteComponent->GetIdArray(DataType::ID_TAG, sprite->m_tags);						
					}
					else if(aSpriteComponent->m_name == "borders")
					{
						std::vector<uint32_t>* borders = &sprite->m_info.m_borders;

						if(aSpriteComponent->m_annotation)
						{
							uint32_t spriteId = aSpriteComponent->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aSpriteComponent->m_annotation->GetIdentifier());
							borders = sprite->m_info.GetOrCreateBorderArray(spriteId);
						}


						aSpriteComponent->GetNullOptionalIdArray(DataType::ID_SPRITE, *borders);
					}
					else if (aSpriteComponent->m_name == "alt_tiles")
					{
						aSpriteComponent->GetIdArray(DataType::ID_SPRITE, sprite->m_info.m_altTileSpriteIds);
					}
					else if (aSpriteComponent->m_name == "water_animation")
					{
						aSpriteComponent->GetIdArray(DataType::ID_SPRITE, sprite->m_info.m_waterAnimationSpriteIds);
					}
					else if(aSpriteComponent->m_name == "origin")
					{
						TP_VERIFY(aSpriteComponent->m_type == SourceNode::TYPE_ARRAY && aSpriteComponent->m_children.size() == 2, aSpriteComponent->m_debugInfo, "Not a valid vector.");
						sprite->m_info.m_origin.m_x = aSpriteComponent->m_children[0]->GetInt32();
						sprite->m_info.m_origin.m_y = aSpriteComponent->m_children[1]->GetInt32();
					}
					else if(aSpriteComponent->m_name == "anchors")
					{
						aSpriteComponent->GetObject()->ForEachChild([&](
							const SourceNode* aAnchor)
						{
							TP_VERIFY(aAnchor->m_type == SourceNode::TYPE_ARRAY && aAnchor->m_children.size() == 4, aAnchor->m_debugInfo, "Not a valid named anchor.");
							SpriteInfo::NamedAnchor t;
							t.m_name = aAnchor->m_name;
							t.m_position.m_x = aAnchor->m_children[0]->GetInt32();
							t.m_position.m_y = aAnchor->m_children[1]->GetInt32();
							t.m_size.m_x = aAnchor->m_children[2]->GetInt32();
							t.m_size.m_y = aAnchor->m_children[3]->GetInt32();
							sprite->m_info.m_namedAnchors.push_back(t);
						});
					}
					else if(aSpriteComponent->m_name == "icon_meta_data")
					{
						SpriteInfo::IconMetaData t;
						aSpriteComponent->GetObject()->ForEachChild([&](
							const SourceNode* aChild)
						{
							if (aChild->m_name == "min_level")
								t.m_minLevel = aChild->GetUInt32();
							else if(aChild->m_name == "max_level")
								t.m_maxLevel = aChild->GetUInt32();
							else if (aChild->m_name == "min_rarity")
								t.m_minRarity = Rarity::StringToId(aChild->GetIdentifier());
							else if (aChild->m_name == "max_rarity")
								t.m_maxRarity = Rarity::StringToId(aChild->GetIdentifier());
							else
								TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						});
						sprite->m_info.m_iconMetaData = t;
					}
					else
					{
						TP_VERIFY(false, aNode->m_debugInfo, "Invalid item in 'sprite'.");
					}
				});

				if(makeAltGreyscale)
				{
					// Make alternate greyscale version of sprite
					Sprite* altGreyscaleSprite = _CreateSprite(NULL, NULL, width * height);
					altGreyscaleSprite->m_image.Copy(sprite->m_image);
					altGreyscaleSprite->m_image.MakeGreyscale(150);

					altGreyscaleSprite->m_info.m_flags = sprite->m_info.m_flags & (SpriteInfo::FLAG_CENTERED | SpriteInfo::FLAG_DOUBLED);

					sprite->m_altGreyscale = altGreyscaleSprite;
				}
			}
			else
			{
				TP_VERIFY(false, aNode->m_debugInfo, "Invalid item in 'sprites'.");
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
	SpriteSheetBuilder::ExportPreliminaryManifestData(
		PersistentIdTable*		aPersistentIdTable,
		Manifest*				aManifest)
	{
		for (SpriteTable::iterator i = m_spriteTable.begin(); i != m_spriteTable.end(); i++)
		{
			Sprite* sprite = i->second.get();
			Data::Sprite* data = aManifest->GetContainer<Data::Sprite>()->GetByName(aPersistentIdTable, sprite->m_name.c_str());

			data->m_width = sprite->m_image.GetWidth();
			data->m_height = sprite->m_image.GetHeight();
			data->m_info = sprite->m_info;
			data->m_tagIds = sprite->m_tags;
			data->m_defined = true;

			sprite->m_data = data;
		}

		// Update alternate greyscale sprite ids
		for (SpriteTable::iterator i = m_spriteTable.begin(); i != m_spriteTable.end(); i++)
		{
			Sprite* sprite = i->second.get();
			if(sprite->m_altGreyscale != NULL)
			{
				assert(sprite->m_altGreyscale->m_data->m_id != 0);
				sprite->m_data->m_info.m_altGreyscaleSpriteId = sprite->m_altGreyscale->m_data->m_id;
			}
		}

	}

	void	
	SpriteSheetBuilder::UpdateManifestData()
	{
		for (SpriteTable::iterator i = m_spriteTable.begin(); i != m_spriteTable.end(); i++)
		{
			Sprite* sprite = i->second.get();

			sprite->m_data->m_offsetX = sprite->m_sheetOffsetX;
			sprite->m_data->m_offsetY = sprite->m_sheetOffsetY;
			sprite->m_data->m_spriteSheetIndex = sprite->m_sheetIndex;
			sprite->m_data->m_index = sprite->m_index;
		}
	}

	void	
	SpriteSheetBuilder::ExportSheets(
		Compression::Level		aCompressionLevel,
		const char*				aPath)
	{
		// Make palette
		std::vector<uint32_t> palette;		
		std::unordered_map<uint32_t, uint32_t> reversePalette;
		uint32_t totalPixelCount = 0;

		{
			std::unordered_map<uint32_t, uint32_t> rgbaCounts;

			for (std::unique_ptr<Sheet>& sheet : m_sheets)
			{
				for (const Sprite* sprite : sheet->m_sprites)
				{
					const Image::RGBA* p = sprite->m_image.GetData();
					for (uint32_t i = 0, count = sprite->m_image.GetWidth() * sprite->m_image.GetHeight(); i < count; i++, p++)
					{
						uint32_t c = *((const uint32_t*)p);
						std::unordered_map<uint32_t, uint32_t>::iterator j = rgbaCounts.find(c);
						if (j == rgbaCounts.end())
							rgbaCounts[c] = 1;
						else
							j->second++;

						TP_CHECK(totalPixelCount < UINT32_MAX, "Total pixel count: too damn high!");
						totalPixelCount++;
					}
				}
			}

			std::multimap<uint32_t, uint32_t> rgbaCountsSorted;
			for (std::unordered_map<uint32_t, uint32_t>::const_iterator i = rgbaCounts.cbegin(); i != rgbaCounts.cend(); i++)
				rgbaCountsSorted.insert(std::make_pair(i->second, i->first));
		
			for(std::multimap<uint32_t, uint32_t>::const_reverse_iterator i = rgbaCountsSorted.crbegin(); i != rgbaCountsSorted.crend(); i++)
			{				
				reversePalette[i->second] = (uint32_t)palette.size();
				palette.push_back(i->second);
			}
		}

		// Serialize to big data blob
		std::vector<uint8_t> compressed;

		{
			std::vector<uint8_t> data;

			_WriteUInt32(data, totalPixelCount);
			_WriteUInt32(data, (uint32_t)palette.size());
			_WriteBuffer(data, &palette[0], palette.size() * sizeof(uint32_t));
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

					// Encode pixels using palette
					const Image::RGBA* p = sprite->m_image.GetData();
					for (uint32_t i = 0, count = sprite->m_image.GetWidth() * sprite->m_image.GetHeight(); i < count; i++, p++)
					{
						uint32_t c = *((const uint32_t*)p);
						std::unordered_map<uint32_t, uint32_t>::const_iterator j = reversePalette.find(c);
						assert(j != reversePalette.cend());
						uint32_t index = j->second;
						_WriteUInt32(data, index);
					}		
				}
			}

			Compression::Pack(&data[0], data.size(), aCompressionLevel, compressed);
		}

		// Write to file
		FILE* f = fopen(aPath, "wb");
		TP_CHECK(f != NULL, "Failed to open file for output: %s", aPath);
		fwrite(&compressed[0], 1, compressed.size(), f);
		fclose(f);
	}

	//---------------------------------------------------------------------------

	SpriteSheetBuilder::Sprite* 
	SpriteSheetBuilder::_CreateSprite(
		const SourceNode*		aNode,
		const char*				aName,
		uint32_t				aSize)
	{
		Sprite* sprite = NULL;

		if(aName == NULL)
		{
			assert(aNode == NULL);
			char dummyName[64];
			TP_STRING_FORMAT(dummyName, sizeof(dummyName), "__unnamed_%u__", m_nextUnnamedIndex++);
			assert(m_spriteTable.find(dummyName) == m_spriteTable.end());
			sprite = new Sprite(dummyName, aSize);
			m_spriteTable[dummyName] = std::unique_ptr<Sprite>(sprite);
		}
		else
		{
			assert(aNode != NULL);
			TP_VERIFY(m_spriteTable.find(aName) == m_spriteTable.end(), aNode->m_debugInfo, "Sprite name '%s' already in use.", aName);
			sprite = new Sprite(aName, aSize);
			m_spriteTable[aName] = std::unique_ptr<Sprite>(sprite);
		}

		return sprite;
	}

	SpriteSheetBuilder::Sprite* 
	SpriteSheetBuilder::_GetSprite(
		const char*				aName)
	{
		SpriteTable::iterator i = m_spriteTable.find(aName);
		assert(i != m_spriteTable.end());
		return i->second.get();
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

		if((aSprite->m_info.m_flags & SpriteInfo::FLAG_STANDALONE) == 0)
		{
			// Find smallest empty space that fits this sprite
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
			if ((aSprite->m_info.m_flags & SpriteInfo::FLAG_STANDALONE) == 0)
			{
				uint32_t nearestPowerOfTwoWidth = _GetNearestPowerOfTwo(width);
				uint32_t nearestPowerOfTwoHeight = _GetNearestPowerOfTwo(height);
				uint32_t sheetSize = std::max(nearestPowerOfTwoWidth, nearestPowerOfTwoHeight);
				sheetSize = std::max(sheetSize, m_minSheetSize);
				insertSheet = _CreateSheet(sheetSize, sheetSize);
			}
			else
			{
				insertSheet = _CreateSheet(width, height);				
			}
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

			aSprite->m_index = (uint32_t)insertSheet->m_sprites.size() - 1;
		}
	}


}