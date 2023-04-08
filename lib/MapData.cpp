#include "Pcheader.h"

#include <kaos-public/Manifest.h>
#include <kaos-public/MapData.h>

#include "Image.h"

namespace kaos_public
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

		return true;
	}

	void	
	MapData::Build(
		const Manifest*			aManifest)
	{
		// Allocate space for tile map
		assert(m_tileMap == NULL);
		m_tileMap = new uint32_t[m_width * m_height];
		for(int32_t i = 0, count = m_width * m_height; i < count; i++)
			m_tileMap[i] = m_defaultTileSpriteId;		

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

}