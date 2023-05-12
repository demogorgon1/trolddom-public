#include "Pcheader.h"

#include <kpublic/Image.h>
#include <kpublic/Manifest.h>
#include <kpublic/MapData.h>
#include <kpublic/TileStackCache.h>

namespace tpublic
{

	namespace
	{

		uint32_t
		_MakeColorKey(
			uint8_t		aR,
			uint8_t		aG, 
			uint8_t		aB)
		{
			return (((uint32_t)aR) << 16) | (((uint32_t)aG) << 8) | ((uint32_t)aB);
		}

	}

	//--------------------------------------------------------------------

	MapData::MapData()
		: m_width(0)
		, m_height(0)
		, m_x(0)
		, m_y(0)
		, m_tileMap(NULL)
		, m_defaultTileSpriteId(0)
		, m_walkableBits(NULL)
		, m_blockLineOfSightBits(NULL)
	{

	}

	MapData::MapData(
		const Parser::Node*		aSource)
		: m_width(0)
		, m_height(0)
		, m_x(0)
		, m_y(0)
		, m_tileMap(NULL)
		, m_defaultTileSpriteId(0)
		, m_defaultPlayerSpawnId(0)
		, m_walkableBits(NULL)
		, m_blockLineOfSightBits(NULL)
	{
		aSource->GetObject()->ForEachChild([&](
			const Parser::Node* aNode)
		{
			if(aNode->m_name == "default_tile")
				m_defaultTileSpriteId = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aNode->GetIdentifier());
			else if (aNode->m_name == "default_player_spawn")
				m_defaultPlayerSpawnId = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_PLAYER_SPAWN, aNode->GetIdentifier());
			else if(aNode->m_name == "image_output")
				m_imageOutputPath = aNode->m_path + "/" + aNode->m_value;
			else if(aNode->m_name == "layers")
				_InitLayers(aNode->GetArray());
			else
				KP_VERIFY(false, aNode->m_debugInfo, "Invalid 'map_data' item.");
		});
	}

	MapData::~MapData()
	{
		if(m_tileMap != NULL)
			delete [] m_tileMap;

		if(m_walkableBits != NULL)
			delete [] m_walkableBits;

		if (m_blockLineOfSightBits != NULL)
			delete[] m_blockLineOfSightBits;
	}

	void	
	MapData::Build(
		const Manifest*			aManifest,
		TileStackCache*			aTileStackCache)
	{
		// Allocate space for tile map
		assert(m_tileMap == NULL);
		m_tileMap = new uint32_t[m_width * m_height];
		for(int32_t i = 0, count = m_width * m_height; i < count; i++)
			m_tileMap[i] = m_defaultTileSpriteId;		

		// Allocate temporary tile stack map
		std::vector<std::vector<uint32_t>> tileStackMap;
		tileStackMap.resize(m_width * m_height);

		// Process layers
		for(std::unique_ptr<SourceLayer>& layer : m_sourceLayers)
		{
			typedef std::unordered_map<uint32_t, const Data::MapPalette::Entry*> CombinedPalette;
			CombinedPalette combinedPalette;

			// Make combined palette
			for(uint32_t paletteId : layer->m_palettes)
			{
				const Data::MapPalette* palette = aManifest->m_mapPalettes.GetById(paletteId);

				for(const Data::MapPalette::Entry& paletteEntry : palette->m_entries)
				{
					uint32_t key = _MakeColorKey(paletteEntry.m_color.m_r, paletteEntry.m_color.m_g, paletteEntry.m_color.m_b);
					KP_VERIFY(combinedPalette.find(key) == combinedPalette.end(), layer->m_debugInfo, "Source color defined by multiple palettes used at the same time.");
					combinedPalette[key] = &paletteEntry;
				}
			}

			// Process image
			{
				const SourceLayer::RGB* src = layer->m_rgb;
				size_t maskIndex = 0;

				for (int32_t y = 0; y < layer->m_height; y++)
				{
					for (int32_t x = 0; x < layer->m_width; x++)
					{
						if ((layer->m_mask[maskIndex / 8] & (1 << (maskIndex % 8))) != 0)
						{
							uint32_t key = _MakeColorKey(src->m_r, src->m_g, src->m_b);
							CombinedPalette::iterator i = combinedPalette.find(key);
							KP_VERIFY(i != combinedPalette.end(), layer->m_debugInfo, "Source color not defined in any palette: %u %u %u (%d, %d)", src->m_r, src->m_g, src->m_b, x, y);

							const Data::MapPalette::Entry* entry = i->second;

							int32_t mapX = x - layer->m_x;
							int32_t mapY = y - layer->m_y;
							assert(mapX >= 0 && mapY >= 0 && mapX < m_width && mapY < m_height);

							switch(entry->m_type)
							{
							case Data::MapPalette::ENTRY_TYPE_TILE:
								m_tileMap[mapX + mapY * m_width] = entry->m_id;
								break;

							case Data::MapPalette::ENTRY_TYPE_ENTITY_SPAWN:
								m_entitySpawns.push_back({ mapX, mapY, entry->m_id });
								break;

							case Data::MapPalette::ENTRY_TYPE_PLAYER_SPAWN:
								m_playerSpawns.push_back({ mapX, mapY, entry->m_id });
								break;

							case Data::MapPalette::ENTRY_TYPE_PORTAL:
								m_portals.push_back({ mapX, mapY, entry->m_id });
								break;

							default:
								assert(false);
								break;
							}
						}

						src++;
						maskIndex++;
					}
				}
			}
		}

		// Make list of tiles with borders
		std::vector<const Data::Sprite*> spritesWithBorders;
		aManifest->m_sprites.ForEach([&](
			const Data::Sprite* aSprite)
		{
			if(aSprite->m_info.m_borders.size() > 0)
				spritesWithBorders.push_back(aSprite);
		});

		// Sort by tile layer 
		std::sort(spritesWithBorders.begin(), spritesWithBorders.end(), [](
			const Data::Sprite* aLHS,
			const Data::Sprite* aRHS)
		{
			return aLHS->m_info.m_tileLayer < aRHS->m_info.m_tileLayer;
		});

		// Generate tile-stack map (tile borders stacked on top of each other)
		for (const Data::Sprite* t : spritesWithBorders)
		{
			KP_VERIFY(t->m_info.m_borders.size() == 17, t->m_debugInfo, "Bordered tile must have 17 borders defined.");

			for (int32_t y = 0; y < m_height; y++)
			{
				for (int32_t x = 0; x < m_width; x++)
				{
					uint32_t tileSpriteId = m_tileMap[x + y * m_width];
					if(tileSpriteId == t->m_id)
						continue;

					const Data::Sprite* sprite = aManifest->m_sprites.GetById(tileSpriteId);
					if(sprite->m_info.m_tileLayer >= t->m_info.m_tileLayer)	
						continue;

					std::vector<uint32_t>& tileStack = tileStackMap[x + y * m_width];					

					// Unleash the tiling monster
					bool bits[8];
					bits[0] = _GetTile(x - 1, y - 1) == t->m_id;
					bits[1] = _GetTile(x, y - 1) == t->m_id;
					bits[2] = _GetTile(x + 1, y - 1) == t->m_id;
					bits[3] = _GetTile(x + 1, y) == t->m_id;
					bits[4] = _GetTile(x + 1, y + 1) == t->m_id;
					bits[5] = _GetTile(x, y + 1) == t->m_id;
					bits[6] = _GetTile(x - 1, y + 1) == t->m_id;
					bits[7] = _GetTile(x - 1, y) == t->m_id;

					if (bits[1] && bits[7] && !bits[3] && !bits[5])
						tileStack.push_back(t->m_info.m_borders[0]);
					if (bits[1] && bits[3] && !bits[5] && !bits[7])
						tileStack.push_back(t->m_info.m_borders[1]);
					if (bits[3] && bits[5] && !bits[7] && !bits[1])
						tileStack.push_back(t->m_info.m_borders[2]);
					if (bits[5] && bits[7] && !bits[1] && !bits[3])
						tileStack.push_back(t->m_info.m_borders[3]);

					if (bits[5] && !bits[7] && !bits[3])
						tileStack.push_back(t->m_info.m_borders[4]);
					if (bits[7] && !bits[1] && !bits[5])
						tileStack.push_back(t->m_info.m_borders[5]);
					if (bits[1] && !bits[7] && !bits[3])
						tileStack.push_back(t->m_info.m_borders[6]);
					if (bits[3] && !bits[1] && !bits[5])
						tileStack.push_back(t->m_info.m_borders[7]);

					if (!bits[1] && bits[3] && bits[5] && bits[7])
						tileStack.push_back(t->m_info.m_borders[8]);
					if (bits[1] && !bits[3] && bits[5] && bits[7])
						tileStack.push_back(t->m_info.m_borders[9]);
					if (bits[1] && bits[3] && !bits[5] && bits[7])
						tileStack.push_back(t->m_info.m_borders[10]);
					if (bits[1] && bits[3] && bits[5] && !bits[7])
						tileStack.push_back(t->m_info.m_borders[11]);

					if (bits[0] && !bits[7] && !bits[1])
						tileStack.push_back(t->m_info.m_borders[12]);
					if (bits[2] && !bits[1] && !bits[3])
						tileStack.push_back(t->m_info.m_borders[13]);
					if (bits[4] && !bits[3] && !bits[5])
						tileStack.push_back(t->m_info.m_borders[14]);
					if (bits[6] && !bits[5] && !bits[7])
						tileStack.push_back(t->m_info.m_borders[15]);

					if (bits[1] && bits[3] && bits[5] && bits[7])
						tileStack.push_back(t->m_info.m_borders[16]);
				}
			}
		}

		// Process tile stacks. See if we need to automatically generate border some tiles.
		for (int32_t y = 0; y < m_height; y++)
		{
			for (int32_t x = 0; x < m_width; x++)
			{
				int32_t i = x + y * m_width;

				const std::vector<uint32_t>& tileStack = tileStackMap[i];

				if(tileStack.size() > 0)
					m_tileMap[i] = aTileStackCache->GetSpriteId(m_tileMap[i], tileStack);				
			}
		}
	}

	void	
	MapData::ToStream(
		IWriter*				aStream) const
	{
		assert(m_tileMap != NULL);

		aStream->WriteUInt(m_defaultTileSpriteId);
		aStream->WriteUInt(m_defaultPlayerSpawnId);
		aStream->WriteInt(m_x);
		aStream->WriteInt(m_y);
		aStream->WriteInt(m_width);
		aStream->WriteInt(m_height);

		for(int32_t i = 0, count = m_width * m_height; i < count; i++)
			aStream->WriteUInt(m_tileMap[i]);

		aStream->WriteObjects(m_entitySpawns);
		aStream->WriteObjects(m_playerSpawns);
		aStream->WriteObjects(m_portals);
	}

	bool	
	MapData::FromStream(
		IReader*				aStream)
	{
		if (!aStream->ReadUInt(m_defaultTileSpriteId))
			return false;
		if (!aStream->ReadUInt(m_defaultPlayerSpawnId))
			return false;
		if (!aStream->ReadInt(m_x))
			return false;
		if (!aStream->ReadInt(m_y))
			return false;
		if (!aStream->ReadInt(m_width))
			return false;
		if (!aStream->ReadInt(m_height))
			return false;

		if(m_width < 0 || m_width > 2048 || m_height < 0 || m_height > 2048)
			return false;

		assert(m_tileMap == NULL);
		m_tileMap = new uint32_t[m_width * m_height];
		for (int32_t i = 0, count = m_width * m_height; i < count; i++)
		{
			if(!aStream->ReadUInt(m_tileMap[i]))
				return false;
		}

		if (!aStream->ReadObjects(m_entitySpawns))
			return false;
		if (!aStream->ReadObjects(m_playerSpawns))
			return false;
		if (!aStream->ReadObjects(m_portals))
			return false;

		return true;
	}

	void	
	MapData::PrepareRuntime(
		const Manifest*			aManifest)
	{
		assert(m_walkableBits == NULL);
		assert(m_blockLineOfSightBits == NULL);
		assert(m_tileMap != NULL);
		assert(m_width > 0 && m_height > 0);

		_InitBits(aManifest);
	}

	bool	
	MapData::IsTileWalkable(
		int32_t					aX,
		int32_t					aY) const
	{
		assert(m_walkableBits != NULL);
		int32_t x = aX - m_x;
		int32_t y = aY - m_y;
		if(x < 0 || y < 0 || x >= m_width || y >= m_height)
			return false;
	
		uint32_t i = (uint32_t)(x + y * m_width);
		uint32_t j = i / 32;
		uint32_t k = i % 32;
		return (m_walkableBits[j] & (1 << k)) != 0;
	}

	bool	
	MapData::DoesTileblockLineOfSight(
		int32_t					aX,
		int32_t					aY) const
	{
		assert(m_blockLineOfSightBits != NULL);
		int32_t x = aX - m_x;
		int32_t y = aY - m_y;
		if(x < 0 || y < 0 || x >= m_width || y >= m_height)
			return false;
	
		uint32_t i = (uint32_t)(x + y * m_width);
		uint32_t j = i / 32;
		uint32_t k = i % 32;
		return (m_blockLineOfSightBits[j] & (1 << k)) != 0;
	}

	//--------------------------------------------------------------------

	void	
	MapData::_InitLayers(
		const Parser::Node*		aLayersArray)
	{		
		int32_t minX = INT32_MAX;
		int32_t minY = INT32_MAX;
		int32_t maxX = INT32_MIN;
		int32_t maxY = INT32_MIN;

		aLayersArray->ForEachChild([&](
			const Parser::Node* aLayer)
		{
			std::unique_ptr<SourceLayer> layer = std::make_unique<SourceLayer>();
			layer->m_debugInfo = aLayer->m_debugInfo;

			std::string sourcePath;

			aLayer->GetObject()->ForEachChild([&](
				const Parser::Node* aNode)
			{
				if(aNode->m_name == "palettes")
				{
					aNode->GetIdArray(DataType::ID_MAP_PALETTE, layer->m_palettes);
				}
				else if(aNode->m_name == "source")
				{
					sourcePath = aNode->m_path + "/" + aNode->m_value;
				}
				else if (aNode->GetArray()->m_name == "offset" && aNode->m_children.size() == 2)
				{
					layer->m_x = aNode->m_children[0]->GetInt32();
					layer->m_y = aNode->m_children[1]->GetInt32();
				}
				else
				{
					KP_VERIFY(false, aNode->m_debugInfo, "Invalid layer item.");
				}
			});

			KP_VERIFY(!sourcePath.empty(), aLayer->m_debugInfo, "Missing 'source' item.");
			KP_VERIFY(layer->m_palettes.size() > 0, aLayer->m_debugInfo, "No palettes defined for layer.");

			Image sourceImage;
			sourceImage.LoadPNG(sourcePath.c_str());

			layer->m_width = (int32_t)sourceImage.GetWidth();
			layer->m_height = (int32_t)sourceImage.GetHeight();

			minX = std::min(minX, layer->m_x);
			minY = std::min(minY, layer->m_y);
			maxX = std::max(maxX, layer->m_x + layer->m_width);
			maxY = std::max(maxY, layer->m_y + layer->m_height);

			layer->m_rgb = new SourceLayer::RGB[sourceImage.GetWidth() * sourceImage.GetHeight()];

			size_t maskSize = sourceImage.GetSize() / 8;
			if(sourceImage.GetSize() % 8 != 0)
				maskSize++;
			layer->m_mask = new uint8_t[maskSize];
			memset(layer->m_mask, 0, maskSize);

			const Image::RGBA* src = sourceImage.GetData();
			SourceLayer::RGB* dest = layer->m_rgb;
			size_t i = 0;
			for(uint32_t y = 0; y < sourceImage.GetHeight(); y++)
			{
				for (uint32_t x = 0; x < sourceImage.GetWidth(); x++)
				{
					if (src->m_a == 255)
					{
						dest->m_r = src->m_r;
						dest->m_g = src->m_g;
						dest->m_b = src->m_b;

						size_t maskIndex = i / 8;
						uint8_t bit = (uint8_t)(i % 8);
						layer->m_mask[maskIndex] |= (1 << bit);
					}
					else
					{
						KP_VERIFY(src->m_a == 0, aLayer->m_debugInfo, "Source alpha channel should be 0 or 255.");
					}

					src++;
					dest++;
					i++;
				}
			}

			m_sourceLayers.push_back(std::move(layer));
		});

		m_x = minX;
		m_y = minY;
		m_width = maxX - minX;
		m_height = maxY - minY;
	}

	void	
	MapData::_InitBits(
		const Manifest*				aManifest)
	{
		int32_t count = m_width * m_height / 32;
		if((m_width * m_height % 32) != 0)
			count++;

		m_walkableBits = new uint32_t[count];
		m_blockLineOfSightBits = new uint32_t[count];

		memset(m_walkableBits, 0, sizeof(uint32_t) * (size_t)count);
		memset(m_blockLineOfSightBits, 0, sizeof(uint32_t) * (size_t)count);

		const uint32_t* in = m_tileMap;
		uint32_t* outWalkable = m_walkableBits;
		uint32_t* outBlockLineOfSight = m_blockLineOfSightBits;
		uint32_t bit = 0;

		for(int32_t y = 0; y < m_height; y++)
		{
			for(int32_t x = 0; x < m_width; x++)
			{
				const Data::Sprite* sprite = aManifest->m_sprites.GetById(*in);
				
				if(sprite->m_info.m_flags & SpriteInfo::FLAG_TILE_WALKABLE)
					*outWalkable |= 1 << bit;

				if (sprite->m_info.m_flags & SpriteInfo::FLAG_TILE_BLOCK_LINE_OF_SIGHT)
					*outBlockLineOfSight |= 1 << bit;

				// Next tile
				in++;
				bit++;

				if(bit == 32)
				{
					bit = 0;
					outWalkable++;
					outBlockLineOfSight++;
				}
			}
		}
	}

	uint32_t	
	MapData::_GetTile(
		int32_t				aX,
		int32_t				aY)
	{
		if(aX >= 0 && aY >= 0 && aX < m_width && aY < m_height)
			return m_tileMap[aX + aY * m_width];

		return 0;
	}

}