#include "Pcheader.h"

#include <tpublic/Data/CliffStyle.h>
#include <tpublic/Data/Doodad.h>
#include <tpublic/Data/MapCliff.h>
#include <tpublic/Data/MapPalette.h>
#include <tpublic/Data/Sprite.h>
#include <tpublic/Data/Wall.h>

#include <tpublic/AutoDoodads.h>
#include <tpublic/CliffBuilder.h>
#include <tpublic/DebugPrintTimer.h>
#include <tpublic/DoodadPlacement.h>
#include <tpublic/Image.h>
#include <tpublic/Manifest.h>
#include <tpublic/MapData.h>
#include <tpublic/MapPathData.h>
#include <tpublic/MapRouteData.h>
#include <tpublic/UniformDistribution.h>
#include <tpublic/WorldInfoMap.h>

#include "MapPathDataBuilder.h"

namespace tpublic
{

	namespace
	{

		uint32_t
		_MakeColorKey(
			uint8_t										aR,
			uint8_t										aG, 
			uint8_t										aB)
		{
			return (((uint32_t)aR) << 16) | (((uint32_t)aG) << 8) | ((uint32_t)aB);
		}

		void
		_WriteStaticPositionToolTipTable(
			IWriter*									aStream,
			const MapData::StaticPositionToolTipTable&	aStaticPositionToolTipTable)
		{
			aStream->WriteUInt(aStaticPositionToolTipTable.size());
			for (MapData::StaticPositionToolTipTable::const_iterator i = aStaticPositionToolTipTable.cbegin(); i != aStaticPositionToolTipTable.cend(); i++)
			{
				i->first.ToStream(aStream);
				aStream->WriteString(i->second);
			}
		}

		void
		_WriteObjectTable(
			IWriter*									aStream,
			const MapData::ObjectTable&					aObjectTable)
		{
			aStream->WriteUInt(aObjectTable.size());
			for (MapData::ObjectTable::const_iterator i = aObjectTable.cbegin(); i != aObjectTable.cend(); i++)
			{
				i->first.ToStream(aStream);
				aStream->WriteUInt(i->second);
			}
		}

		bool
		_ReadObjectTable(
			IReader*									aStream,
			MapData::ObjectTable&						aObjectTable)
		{
			size_t count;
			if (!aStream->ReadUInt(count))
				return false;
			for (size_t i = 0; i < count; i++)
			{
				Vec2 position;
				if (!position.FromStream(aStream))
					return false;
				uint32_t id;
				if (!aStream->ReadUInt(id))
					return false;
				aObjectTable[position] = id;
			}
			return true;
		}

		bool
		_ReadStaticPositionToolTipTable(
			IReader*									aStream,
			MapData::StaticPositionToolTipTable&		aStaticPositionToolTipTable)
		{
			size_t count;
			if (!aStream->ReadUInt(count))
				return false;
			for (size_t i = 0; i < count; i++)
			{
				Vec2 position;
				if (!position.FromStream(aStream))
					return false;
				std::string t;
				if (!aStream->ReadString(t))
					return false;
				aStaticPositionToolTipTable[position] = std::move(t);
			}
			return true;
		}

		uint32_t
		_GetObject(
			const MapData::ObjectTable&					aObjectTable,
			const Vec2&									aPosition)
		{
			MapData::ObjectTable::const_iterator i = aObjectTable.find(aPosition);
			if(i != aObjectTable.cend())
				return i->second;
			return 0;
		}
	}

	//--------------------------------------------------------------------

	MapData::MapData()
		: m_width(0)
		, m_height(0)
		, m_x(0)
		, m_y(0)
		, m_tileMap(NULL)
		, m_elevationMap(NULL)
		, m_walkableBits(NULL)
		, m_blockLineOfSightBits(NULL)
		, m_alwaysObscuredBits(NULL)
		, m_resetMode(MapType::RESET_MODE_MANUAL)
		, m_type(MapType::ID_OPEN_WORLD)
	{

	}

	MapData::MapData(
		const SourceNode*		aSource)
		: m_width(0)
		, m_height(0)
		, m_type(MapType::ID_OPEN_WORLD)
		, m_resetMode(MapType::RESET_MODE_MANUAL)
		, m_x(0)
		, m_y(0)
		, m_tileMap(NULL)
		, m_elevationMap(NULL)
		, m_walkableBits(NULL)
		, m_alwaysObscuredBits(NULL)
		, m_blockLineOfSightBits(NULL)
	{
		aSource->GetObject()->ForEachChild([&](
			const SourceNode* aNode)
		{
			if(!m_mapInfo.FromSourceItem(aNode))
			{
				if(aNode->m_name == "image_output")
					m_imageOutputPath = aNode->m_path + "/" + aNode->m_value;
				else if(aNode->m_name == "layers")
					_InitLayers(aNode->GetArray());
				else if(aNode->m_name == "type")
					m_type = MapType::StringToId(aNode->GetIdentifier());
				else if (aNode->m_name == "reset")
					m_resetMode = MapType::StringToResetMode(aNode->GetIdentifier());
				else if(aNode->m_name == "script")
					m_scripts.push_back(std::make_unique<Script>(aNode));
				else if(aNode->m_tag == "generator")
					m_generator = std::make_unique<Generator>(aNode);
				else if(aNode->m_name == "seed")
					m_seed.FromSource(aNode);
				else if(aNode->m_name == "pvp")
					m_pvp = std::make_unique<PVP>(aNode);
				else if(aNode->m_name == "realm_balance")
					aNode->GetIdArray(DataType::ID_REALM_BALANCE, m_realmBalanceIds);
				else
					TP_VERIFY(false, aNode->m_debugInfo, "'%s' is not a valid item.", aNode->m_name.c_str());
			}
		});
	}

	MapData::~MapData()
	{
		if(m_tileMap != NULL)
			delete [] m_tileMap;

		if(m_walkableBits != NULL)
			delete [] m_walkableBits;

		if (m_blockLineOfSightBits != NULL)
			delete [] m_blockLineOfSightBits;

		if(m_alwaysObscuredBits != NULL)
			delete [] m_alwaysObscuredBits;

		if(m_elevationMap != NULL)
			delete [] m_elevationMap;
	}

	void	
	MapData::Build(
		const Manifest*			aManifest,
		const AutoDoodads*		aAutoDoodads,
		nwork::Queue*			/*aWorkQueue*/)
	{
		std::mt19937 random;

		if(m_sourceLayers.size() > 0)
		{
			// Allocate space for tile map and elevation map
			assert(m_tileMap == NULL);
			assert(m_elevationMap == NULL);
			m_tileMap = new uint32_t[m_width * m_height];
			m_elevationMap = new uint8_t[m_width * m_height];
			for(int32_t i = 0, count = m_width * m_height; i < count; i++)
			{
				m_tileMap[i] = m_mapInfo.m_defaultTileSpriteId;
				m_elevationMap[i] = 0;
			}

			// Allocate space for temporary level and zone maps
			std::vector<uint32_t> levelMap;
			levelMap.resize(m_width * m_height, 0);
			bool hasLevelMap = false;

			std::vector<uint32_t> zoneMap;
			zoneMap.resize(m_width * m_height, 0);
			bool hasZoneMap = false;

			std::vector<uint32_t> coverMap;
			coverMap.resize(m_width * m_height, 0);
			bool hasCoverMap = false;

			std::vector<uint32_t> subZoneMap;
			subZoneMap.resize(m_width * m_height, 0);
			bool hasSubZoneMap = false;

			std::vector<uint8_t> flagsMap;
			flagsMap.resize(m_width * m_height, 0);
			bool hasFlagsMap = false;

			std::unique_ptr<CliffBuilder> cliffBuilder;

			std::unordered_map<Vec2, uint32_t, Vec2::Hasher> doodadCoverageMap;

			// Process layers
			for(std::unique_ptr<SourceLayer>& layer : m_sourceLayers)
			{
				typedef std::unordered_map<uint32_t, const Data::MapPalette::Entry*> CombinedPalette;
				CombinedPalette combinedPalette;

				// Make combined palette
				for(uint32_t paletteId : layer->m_palettes)
				{
					const Data::MapPalette* palette = aManifest->GetById<Data::MapPalette>(paletteId);

					for(const Data::MapPalette::Entry& paletteEntry : palette->m_entries)
					{
						uint32_t key = _MakeColorKey(paletteEntry.m_color.m_r, paletteEntry.m_color.m_g, paletteEntry.m_color.m_b);
						TP_VERIFY(combinedPalette.find(key) == combinedPalette.end(), layer->m_debugInfo, "Source color defined by multiple palettes used at the same time.");
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
								TP_VERIFY(i != combinedPalette.end(), layer->m_debugInfo, "Source color not defined in any palette: %u %u %u (%d, %d)", src->m_r, src->m_g, src->m_b, x, y);

								const Data::MapPalette::Entry* entry = i->second;

								int32_t mapX = x - layer->m_x;
								int32_t mapY = y - layer->m_y;
								assert(mapX >= 0 && mapY >= 0 && mapX < m_width && mapY < m_height);

								if(layer->m_cover)
								{
									TP_VERIFY(entry->m_type == Data::MapPalette::ENTRY_TYPE_TILE, layer->m_debugInfo, "Cover layer can only have tiles.");
									coverMap[mapX + mapY * m_width] = entry->m_value;
									hasCoverMap = true;
								}
								else
								{
									if(!entry->m_string.empty())
										m_staticPositionToolTips[{ mapX, mapY }] = entry->m_string;

									switch(entry->m_type)
									{
									case Data::MapPalette::ENTRY_TYPE_TILE:
										m_tileMap[mapX + mapY * m_width] = entry->m_value;
										break;

									case Data::MapPalette::ENTRY_TYPE_ENTITY_SPAWN:
										m_entitySpawns.push_back({ mapX, mapY, entry->m_value });
										break;

									case Data::MapPalette::ENTRY_TYPE_PLAYER_SPAWN:
										m_playerSpawns.push_back({ mapX, mapY, entry->m_value });
										break;

									case Data::MapPalette::ENTRY_TYPE_PORTAL:
										m_portals.push_back({ mapX, mapY, entry->m_value });
										break;

									case Data::MapPalette::ENTRY_TYPE_LEVEL:
										levelMap[mapX + mapY * m_width] = entry->m_value;
										hasLevelMap = true;
										break;

									case Data::MapPalette::ENTRY_TYPE_ZONE:
										zoneMap[mapX + mapY * m_width] = entry->m_value;
										hasZoneMap = true;
										break;

									case Data::MapPalette::ENTRY_TYPE_SUB_ZONE:
										subZoneMap[mapX + mapY * m_width] = entry->m_value;
										hasSubZoneMap = true;
										break;

									case Data::MapPalette::ENTRY_TYPE_WALL:
										m_walls[{ mapX, mapY }] = entry->m_value;
										break;

									case Data::MapPalette::ENTRY_TYPE_ROUTE:
										{
											if (!m_mapRouteData)
												m_mapRouteData = std::make_unique<MapRouteData>();
											m_mapRouteData->GetOrCreateRoute(entry->m_value)->m_positions.insert({ mapX, mapY });
										}
										break;

									case Data::MapPalette::ENTRY_TYPE_DOODAD:
										{
											const Data::Doodad* doodad = aManifest->GetById<Data::Doodad>(entry->m_value);
											if(doodad->m_spriteIds.size() > 0)
											{
												DoodadPlacement::AddToCoverageMap(doodad, { mapX, mapY }, doodadCoverageMap);
												UniformDistribution<size_t> distribution(0, doodad->m_spriteIds.size() - 1);
												m_doodads[{ mapX, mapY }] = doodad->m_spriteIds[distribution(random)];
											}
										}
										break;

									case Data::MapPalette::ENTRY_TYPE_FLAGS:
										flagsMap[mapX + mapY * m_width] = (uint8_t)entry->m_value;
										hasFlagsMap = true;
										break;

									case Data::MapPalette::ENTRY_TYPE_CLIFF:
										{
											if(!cliffBuilder)
												cliffBuilder = std::make_unique<CliffBuilder>();

											const Data::MapCliff* mapCliff = aManifest->GetById<Data::MapCliff>(entry->m_value);
											const Data::CliffStyle* cliffStyle = aManifest->GetById<Data::CliffStyle>(mapCliff->m_cliffStyleId);
											cliffBuilder->AddCliff({ mapX, mapY }, cliffStyle, mapCliff->m_elevation, mapCliff->m_ramp);
											m_elevationMap[mapX + mapY * m_width] = mapCliff->m_elevation;
										}
										break;

									default:
										assert(false);
										break;
									}
								}
							}

							src++;
							maskIndex++;
						}
					}
				}
			}

			{
				DebugPrintTimer debugPrintTimer("build world info map");

				if(m_mapInfo.m_autoIndoor && hasCoverMap)
				{
					WorldInfoMap::AutoIndoor(aManifest, m_width, m_height, &coverMap[0], &flagsMap[0]);

					hasFlagsMap = true;
				}

				if (hasLevelMap || hasZoneMap || hasSubZoneMap || hasFlagsMap)
				{
					m_worldInfoMap = std::make_unique<WorldInfoMap>();
					m_worldInfoMap->Build(aManifest, m_width, m_height, &levelMap[0], &zoneMap[0], &subZoneMap[0], &flagsMap[0]);
				}
			}

			if(hasCoverMap)
			{
				DebugPrintTimer debugPrintTimer("build cover map");

				m_mapCovers = std::make_unique<MapCovers>();
				m_mapCovers->Build(m_width, m_height, &coverMap[0]);
			}

			if(m_mapInfo.m_autoDoodads)
			{
				DebugPrintTimer debugPrintTimer("generate doodads");

				aAutoDoodads->GenerateDoodads(0, m_tileMap, { m_width, m_height }, { 0, 0 }, { m_width, m_height }, doodadCoverageMap, [&](
					const Vec2& aPosition,
					uint32_t	/*aDoodadId*/,
					uint32_t	aSpriteId)
				{
					m_doodads[aPosition] = aSpriteId;
				});
			}

			if(cliffBuilder)
			{
				DebugPrintTimer debugPrintTimer("generate cliffs");

				cliffBuilder->Generate([&](
					const CliffBuilder::Tile& aTile)
				{
					m_doodads[aTile.m_position] = aTile.m_spriteId;
				});
			}
		}
	}

	void	
	MapData::ConstructMapPathData(
		const Manifest*			aManifest,
		nwork::Queue*			aWorkQueue)
	{		
		m_mapPathData = std::make_unique<MapPathData>();

		MapPathDataBuilder builder;

		{
			DebugPrintTimer debugPrintTimer("build map path data");

			builder.Build(aManifest, m_tileMap, (uint32_t)m_width, (uint32_t)m_height, 6, aWorkQueue);
		}

		{
			DebugPrintTimer debugPrintTimer("export map path data");

			builder.Export(m_mapPathData.get());
		}
	}

	void
	MapData::ConstructMapRouteData(
		const Manifest*			aManifest,
		nwork::Queue*			aWorkQueue)
	{
		if(m_mapRouteData)
		{
			DebugPrintTimer debugPrintTimer("build map route data");

			m_mapRouteData->Build(aManifest, m_tileMap, m_width, m_height, aWorkQueue, [&](
				uint32_t aMapEntitySpawnId) -> Vec2
			{
				std::optional<Vec2> result;
				for(const EntitySpawn& entitySpawn : m_entitySpawns)
				{
					if(entitySpawn.m_mapEntitySpawnId == aMapEntitySpawnId)
					{
						result = Vec2{ entitySpawn.m_x, entitySpawn.m_y };
						break;
					}						
				}
				TP_CHECK(result.has_value(), "Map entity spawn id not found in map: %u", aMapEntitySpawnId);
				return result.value();
			});
		}
	}

	void	
	MapData::ToStream(
		IWriter*				aStream) const
	{
		m_mapInfo.ToStream(aStream);
		aStream->WritePOD(m_type);
		aStream->WritePOD(m_resetMode);
		aStream->WriteInt(m_x);
		aStream->WriteInt(m_y);
		aStream->WriteInt(m_width);
		aStream->WriteInt(m_height);

		for(int32_t i = 0, count = m_width * m_height; i < count; i++)
			aStream->WriteUInt(m_tileMap[i]);

		aStream->WriteObjects(m_entitySpawns);
		aStream->WriteObjects(m_playerSpawns);
		aStream->WriteObjects(m_portals);
		aStream->WriteObjectPointers(m_scripts);
		aStream->WriteObjectPointer(m_mapPathData);
		aStream->WriteOptionalObjectPointer(m_generator);
		m_seed.ToStream(aStream);
		aStream->WriteOptionalObjectPointer(m_worldInfoMap);
		aStream->WriteOptionalObjectPointer(m_mapCovers);
		_WriteObjectTable(aStream, m_doodads);
		_WriteObjectTable(aStream, m_walls);
		aStream->WriteOptionalObjectPointer(m_mapRouteData);
		aStream->Write(m_elevationMap, m_width * m_height);
		_WriteStaticPositionToolTipTable(aStream, m_staticPositionToolTips);
		aStream->WriteOptionalObjectPointer(m_pvp);
		aStream->WriteUInts(m_realmBalanceIds);
	}

	bool	
	MapData::FromStream(
		IReader*				aStream)
	{
		if(!m_mapInfo.FromStream(aStream))
			return false;
		if (!aStream->ReadPOD(m_type))
			return false;
		if (!aStream->ReadPOD(m_resetMode))
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

		if(m_width != 0 && m_height != 0)
		{
			assert(m_tileMap == NULL);
			m_tileMap = new uint32_t[m_width * m_height];
			for (int32_t i = 0, count = m_width * m_height; i < count; i++)
			{
				if(!aStream->ReadUInt(m_tileMap[i]))
					return false;
			}
		}

		if (!aStream->ReadObjects(m_entitySpawns, 32768))
			return false;
		if (!aStream->ReadObjects(m_playerSpawns))
			return false;
		if (!aStream->ReadObjects(m_portals))
			return false;
		if(!aStream->ReadObjectPointers(m_scripts))
			return false;
		if(!aStream->ReadObjectPointer(m_mapPathData))
			return false;
		if(!aStream->ReadOptionalObjectPointer(m_generator))
			return false;
		if(!m_seed.FromStream(aStream))
			return false;
		if (!aStream->ReadOptionalObjectPointer(m_worldInfoMap))
			return false;
		if (!aStream->ReadOptionalObjectPointer(m_mapCovers))
			return false;
		if (!_ReadObjectTable(aStream, m_doodads))
			return false;
		if (!_ReadObjectTable(aStream, m_walls))
			return false;
		if(!aStream->ReadOptionalObjectPointer(m_mapRouteData))
			return false;

		if (m_width != 0 && m_height != 0)
		{
			assert(m_elevationMap == NULL);
			m_elevationMap = new uint8_t[m_width * m_height];
			if(aStream->Read(m_elevationMap, (size_t)m_width * m_height) != (size_t)(m_width * m_height))
				return false;
		}

		if (!_ReadStaticPositionToolTipTable(aStream, m_staticPositionToolTips))
			return false;

		if(!aStream->ReadOptionalObjectPointer(m_pvp))
			return false;
		if(!aStream->ReadUInts(m_realmBalanceIds))
			return false;

		return true;
	}

	void	
	MapData::PrepareRuntime(
		uint8_t					aRuntime,
		const Manifest*			aManifest)
	{
		if(m_tileMap == NULL)
			return;

		if(m_walkableBits != NULL)
		{
			delete[] m_walkableBits;
			m_walkableBits = NULL;
		}

		if(m_blockLineOfSightBits != NULL)
		{
			delete[] m_blockLineOfSightBits;
			m_blockLineOfSightBits = NULL;
		}

		if(m_alwaysObscuredBits != NULL)
		{
			delete[] m_alwaysObscuredBits;
			m_alwaysObscuredBits = NULL;
		}

		assert(m_width > 0 && m_height > 0);

		_InitBits(aManifest);

		if(m_mapRouteData && (aRuntime & DataBase::RUNTIME_SERVER) != 0)
			m_mapRouteData->Prepare();
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
	MapData::IsTileIndoor(
		int32_t					aX,
		int32_t					aY) const
	{
		if(m_worldInfoMap)
			return (m_worldInfoMap->Get({ aX, aY }).m_flags & WorldInfoMap::FLAG_INDOOR) != 0;

		return m_mapInfo.m_defaultIndoor;
	}

	int32_t
	MapData::TraceWalkableTiles(
		const Vec2&				aPosition,
		const Vec2&				aDirection,
		int32_t					aMaxDistance) const
	{
		Vec2 p = aPosition;
		int32_t distance = 0;
		while(IsTileWalkable(p.m_x, p.m_y) && distance < aMaxDistance)
		{
			p.m_x += aDirection.m_x;
			p.m_y += aDirection.m_y;
			distance++;
		}

		return distance;
	}

	uint8_t
	MapData::GetElevation(
		int32_t					aX,
		int32_t					aY) const
	{
		if (m_elevationMap == NULL || aX < 0 || aY < 0 || aX >= m_width || aY >= m_height)
			return 0;

		return m_elevationMap[aX + aY * m_width];
	}

	bool	
	MapData::DoesTileBlockLineOfSight(
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
		if(m_blockLineOfSightBits[j] & (1 << k))
			return true;

		return false;
	}

	bool	
	MapData::IsTileAlwaysObscured(
		int32_t					aX,
		int32_t					aY) const
	{
		assert(m_alwaysObscuredBits != NULL);
		int32_t x = aX - m_x;
		int32_t y = aY - m_y;
		if(x < 0 || y < 0 || x >= m_width || y >= m_height)
			return false;
	
		uint32_t i = (uint32_t)(x + y * m_width);
		uint32_t j = i / 32;
		uint32_t k = i % 32;
		return (m_alwaysObscuredBits[j] & (1 << k)) != 0;
	}

	bool	
	MapData::DoesNeighborTileBlockLineOfSight(
		int32_t					aX,
		int32_t					aY) const
	{
		return DoesTileBlockLineOfSight(aX - 1, aY)
			|| DoesTileBlockLineOfSight(aX + 1, aY)
			|| DoesTileBlockLineOfSight(aX, aY - 1)
			|| DoesTileBlockLineOfSight(aX, aY + 1);
	}

	void	
	MapData::CopyFrom(
		const MapData*			aMapData)
	{
		// FIXME: wonky bonky, need a slam with the refactoring hammer
		m_type = aMapData->m_type;
		m_resetMode = aMapData->m_resetMode;
		m_mapInfo = aMapData->m_mapInfo;

		m_entitySpawns = aMapData->m_entitySpawns;
		m_playerSpawns = aMapData->m_playerSpawns;
		m_portals = aMapData->m_portals;
	
		for(const std::unique_ptr<Script>& script : aMapData->m_scripts)
		{
			std::unique_ptr<Script> t = std::make_unique<Script>();
			*t = *script;
			m_scripts.push_back(std::move(t));
		}

		if(aMapData->m_tileMap != NULL)
		{
			assert(m_tileMap == NULL);
			m_width = aMapData->m_width;
			m_height = aMapData->m_height;
			m_x = aMapData->m_x;
			m_y = aMapData->m_y;
			m_tileMap = new uint32_t[m_width * m_height];
			memcpy(m_tileMap, aMapData->m_tileMap, m_width * m_height * sizeof(uint32_t));
		}

		if(aMapData->m_elevationMap != NULL)
		{
			assert(m_elevationMap == NULL);
			m_elevationMap = new uint8_t[m_width * m_height];
			memcpy(m_elevationMap, aMapData->m_elevationMap, m_width * m_height);
		}

		if(aMapData->m_worldInfoMap)
		{
			assert(!m_worldInfoMap);

			m_worldInfoMap = std::make_unique<WorldInfoMap>();
			m_worldInfoMap->CopyFrom(aMapData->m_worldInfoMap.get());
		}

		if(aMapData->m_mapCovers)
		{
			assert(!m_mapCovers);

			m_mapCovers = std::make_unique<MapCovers>();
			m_mapCovers->CopyFrom(aMapData->m_mapCovers.get());
		}

		if(aMapData->m_mapPathData)
		{
			assert(!m_mapPathData);

			m_mapPathData = std::make_unique<MapPathData>();
			m_mapPathData->CopyFrom(aMapData->m_mapPathData.get());
		}

		if (aMapData->m_mapRouteData)
		{
			assert(!m_mapRouteData);

			m_mapRouteData = std::make_unique<MapRouteData>();
			m_mapRouteData->CopyFrom(aMapData->m_mapRouteData.get());
		}

		m_doodads = aMapData->m_doodads;
		m_walls = aMapData->m_walls;
		m_staticPositionToolTips = aMapData->m_staticPositionToolTips;
	}

	uint32_t	
	MapData::GetTile(
		const Vec2&				aPosition) const
	{
		Vec2 p = aPosition - Vec2{ m_x, m_y };
		if(p.m_x >= 0 && p.m_y >= 0 && p.m_x < m_width && p.m_y < m_height)
			return m_tileMap[p.m_x + p.m_y * m_width];

		return 0;
	}

	void		
	MapData::WriteDebugTileMapPNG(
		const Manifest*			aManifest,
		const char*				aPath) const
	{	
		Image image;
		image.Allocate((uint32_t)m_width, (uint32_t)m_height);

		Image::RGBA* out = image.GetData();

		for(int32_t i = 0; i < m_width * m_height; i++)
		{
			const Data::Sprite* sprite = aManifest->GetById<Data::Sprite>(m_tileMap[i]);
			*out = sprite->m_info.m_averageColor;
			out++;
		}

		image.SavePNG(aPath);
	}

	uint32_t	
	MapData::GetDoodad(
		const Vec2&				aPosition) const
	{
		return _GetObject(m_doodads, aPosition);
	}
	
	uint32_t	
	MapData::GetWall(
		const Vec2&				aPosition) const
	{
		return _GetObject(m_walls, aPosition);
	}

	const char* 
	MapData::GetStaticPositionToolTip(
		const Vec2&				aPosition) const
	{
		StaticPositionToolTipTable::const_iterator i = m_staticPositionToolTips.find(aPosition);
		if(i == m_staticPositionToolTips.cend())
			return NULL;
		return i->second.c_str();
	}

	const MapData::PlayerSpawn* 
	MapData::GetPlayerSpawn(
		uint32_t				aMapPlayerSpawnId) const
	{
		for(const PlayerSpawn& t : m_playerSpawns)
		{
			if(t.m_id == aMapPlayerSpawnId)
				return &t;
		}
		return NULL;
	}

	//--------------------------------------------------------------------

	void	
	MapData::_InitLayers(
		const SourceNode*		aLayersArray)
	{		
		int32_t minX = INT32_MAX;
		int32_t minY = INT32_MAX;
		int32_t maxX = INT32_MIN;
		int32_t maxY = INT32_MIN;

		aLayersArray->ForEachChild([&](
			const SourceNode* aLayer)
		{
			std::unique_ptr<SourceLayer> layer = std::make_unique<SourceLayer>();
			layer->m_debugInfo = aLayer->m_debugInfo;

			std::string sourcePath;

			aLayer->GetObject()->ForEachChild([&](
				const SourceNode* aNode)
			{
				if(aNode->m_name == "palettes")
				{
					aNode->GetIdArray(DataType::ID_MAP_PALETTE, layer->m_palettes);
				}
				else if(aNode->m_name == "source")
				{
					sourcePath = aNode->m_realPath + "/" + aNode->m_value;
				}
				else if (aNode->m_name == "offset" && aNode->GetArray()->m_children.size() == 2)
				{
					layer->m_x = aNode->m_children[0]->GetInt32();
					layer->m_y = aNode->m_children[1]->GetInt32();
				}
				else if (aNode->m_name == "cover")
				{
					layer->m_cover = aNode->GetBool();
				}
				else
				{
					TP_VERIFY(false, aNode->m_debugInfo, "Invalid layer item.");
				}
			});

			TP_VERIFY(!sourcePath.empty(), aLayer->m_debugInfo, "Missing 'source' item.");
			TP_VERIFY(layer->m_palettes.size() > 0, aLayer->m_debugInfo, "No palettes defined for layer.");

			Image sourceImage;
			sourceImage.LoadPNG(sourcePath.c_str());

			layer->m_width = (int32_t)sourceImage.GetWidth();
			layer->m_height = (int32_t)sourceImage.GetHeight();

			minX = Base::Min(minX, layer->m_x);
			minY = Base::Min(minY, layer->m_y);
			maxX = Base::Max(maxX, layer->m_x + layer->m_width);
			maxY = Base::Max(maxY, layer->m_y + layer->m_height);

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
						TP_VERIFY(src->m_a == 0, aLayer->m_debugInfo, "Source alpha channel should be 0 or 255.");
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
		m_alwaysObscuredBits = new uint32_t[count];

		memset(m_walkableBits, 0, sizeof(uint32_t) * (size_t)count);
		memset(m_blockLineOfSightBits, 0, sizeof(uint32_t) * (size_t)count);
		memset(m_alwaysObscuredBits, 0, sizeof(uint32_t) * (size_t)count);

		{
			const uint32_t* in = m_tileMap;
			uint32_t* outWalkable = m_walkableBits;
			uint32_t* outBlockLineOfSight = m_blockLineOfSightBits;
			uint32_t bit = 0;

			for (int32_t y = 0; y < m_height; y++)
			{
				for (int32_t x = 0; x < m_width; x++)
				{
					bool nonWalkableDoodad = false;

					{
						uint32_t doodadSpriteId = GetDoodad({ x, y });
						if(doodadSpriteId != 0)
						{
							const Data::Sprite* doodadSprite = aManifest->GetById<tpublic::Data::Sprite>(doodadSpriteId);
							nonWalkableDoodad = (doodadSprite->m_info.m_flags & SpriteInfo::FLAG_TILE_WALKABLE) == 0;

							// FIXME: support larger non-walkable doodads
						}
					}

					uint32_t wallId = GetWall({ x, y });
					bool wallNotWalkable = false;
					bool wallBlockLineOfSight = false;
					if(wallId != 0)
					{
						const Data::Wall* wall = aManifest->GetById<Data::Wall>(wallId);
						if(wall->m_flags & Data::Wall::FLAG_BLOCK_LINE_OF_SIGHT)
							wallBlockLineOfSight = true;
						if ((wall->m_flags & Data::Wall::FLAG_WALKABLE) == 0)
							wallNotWalkable = true;
					}

					const Data::Sprite* sprite = aManifest->GetById<tpublic::Data::Sprite>(*in);

					if (!wallNotWalkable && !nonWalkableDoodad && (sprite->m_info.m_flags & SpriteInfo::FLAG_TILE_WALKABLE))
						*outWalkable |= 1 << bit;

					if (wallBlockLineOfSight || (sprite->m_info.m_flags & SpriteInfo::FLAG_TILE_BLOCK_LINE_OF_SIGHT))
						*outBlockLineOfSight |= 1 << bit;

					// Next tile
					in++;
					bit++;

					if (bit == 32)
					{
						bit = 0;
						outWalkable++;
						outBlockLineOfSight++;
					}
				}
			}
		}

		{
			uint32_t* out = m_alwaysObscuredBits;
			uint32_t bit = 0;

			// If a tile blocks line of sight and is also completely surrounded by line of sight blockers, flag it as "always obscured"
			for (int32_t y = 0; y < m_height; y++)
			{
				for (int32_t x = 0; x < m_width; x++)
				{
					bool obscured = true;

					for(int32_t yi = -1; yi <= 1 && obscured; yi++)
					{
						for (int32_t xi = -1; xi <= 1 && obscured; xi++)
						{
							if (!DoesTileBlockLineOfSight(x + xi, y + yi))
								obscured = false;
						}
					}

					if(obscured)
						*out |= 1 << bit;

					// Next tile
					bit++;

					if (bit == 32)
					{
						bit = 0;
						out++;
					}
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