#include "../Pcheader.h"

#include <tpublic/Data/Noise.h>
#include <tpublic/Data/Terrain.h>

#include <tpublic/MapGenerators/World.h>

#include <tpublic/DistanceField.h>
#include <tpublic/Helpers.h>
#include <tpublic/Image.h>
#include <tpublic/Manifest.h>
#include <tpublic/MapData.h>
#include <tpublic/NoiseInstance.h>

namespace tpublic::MapGenerators
{

	//---------------------------------------------------------------------------------

	World::World()
		: MapGeneratorBase(ID)
	{

	}
	
	World::~World()
	{

	}

	//---------------------------------------------------------------------------------

	void		
	World::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if(aChild->m_name == "terrain_palette_entry")
					_AddTerrainPaletteEntry(aChild);
				else if(aChild->m_tag == "execute")
					m_executes.push_back(std::make_unique<Execute>(aChild));
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			}
		});
	}
	
	void		
	World::ToStream(
		IWriter*						aWriter) const
	{
		ToStreamBase(aWriter);

		{
			aWriter->WriteUInt(m_terrainPalette.size());
			for(TerrainPalette::const_iterator i = m_terrainPalette.cbegin(); i != m_terrainPalette.cend(); i++)
			{
				aWriter->WritePOD(i->first);
				aWriter->WriteUInt(i->second);
			}
		}

		aWriter->WriteObjectPointers(m_executes);
	}
	
	bool		
	World::FromStream(
		IReader*						aReader) 
	{
		if(!FromStreamBase(aReader))
			return false;

		{
			size_t count;
			if(!aReader->ReadUInt(count))
				return false;

			for(size_t i = 0; i < count; i++)
			{
				char symbol;
				if(!aReader->ReadPOD(symbol))
					return false;

				uint32_t terrainId;
				if(!aReader->ReadUInt(terrainId))
					return false;

				m_terrainPalette[symbol] = terrainId;
			}
		}

		if(!aReader->ReadObjectPointers(m_executes))
			return false;
		return true;
	}

	bool		
	World::Build(
		const Manifest*					aManifest,
		uint32_t						aSeed,
		const MapData*					aSourceMapData,
		const char*						/*aDebugImagePath*/,
		std::unique_ptr<MapData>&		aOutMapData) const 
	{
		printf("BUILD SEED %u\n", aSeed);

		Builder builder;
		builder.m_terrainPalette = &m_terrainPalette;
		builder.m_manifest = aManifest;
		builder.m_random.seed(aSeed);

		for(const std::unique_ptr<Execute>& execute : m_executes)
			builder.Process(execute.get());

		builder.IdentifyWalkableAreas();

		if(builder.m_walkableAreas.size() == 0)
			return false; // This should be very unlikely

		builder.InitNoiseMap();
		builder.ConnectWalkableAreas();

		builder.CreateMapData(aSourceMapData, aOutMapData);

		aOutMapData->WriteDebugTileMapPNG(aManifest, "mapdata.png");

		return true;
	}

	//---------------------------------------------------------------------------------

	void		
	World::_AddTerrainPaletteEntry(
		const SourceNode*				aSource)
	{
		TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing terrain annotation.");
		uint32_t terrainId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TERRAIN, aSource->m_annotation->GetIdentifier());
		char symbol = aSource->GetCharacter();
		TP_VERIFY(!m_terrainPalette.contains(symbol), aSource->m_debugInfo, "'%c' is already defined in terrain palette.");
		m_terrainPalette[symbol] = terrainId;
	}

	//---------------------------------------------------------------------------------

	void		
	World::Builder::Process(
		const Execute*					aExecute)
	{
		switch(aExecute->m_type)
		{
		case EXECUTE_TYPE_ALL:	
			for(const std::unique_ptr<Execute>& child : aExecute->m_children)
			{
				if (MaybeDoIt(child->m_weight))
					Process(child.get());
			}
			break;

		case EXECUTE_TYPE_ONE_OF:	
			if(aExecute->m_children.size() > 0)
			{
				uint32_t roll = Roll(1, aExecute->m_totalWeightOfChildren);
				uint32_t sum = 0;
				const Execute* execute = NULL;
				for (const std::unique_ptr<Execute>& child : aExecute->m_children)
				{
					sum += child->m_weight;
					if(roll <= sum)
					{
						execute = child.get();
						break;
					}
				}

				assert(execute != NULL);

				Process(execute);
			}
			break;

		case EXECUTE_TYPE_PALETTED_MAP:
			m_width = aExecute->m_palettedMap->m_width;
			m_height = aExecute->m_palettedMap->m_height;
			m_terrainMap.resize(aExecute->m_palettedMap->m_tiles.size());
			for(size_t i = 0; i < m_terrainMap.size(); i++)
				m_terrainMap[i] = GetTerrainPaletteEntry(aExecute->m_palettedMap->m_tiles[i]);
			break;

		case EXECUTE_TYPE_DOUBLE:
			for(uint32_t i = 0; i < aExecute->m_value; i++)
				Double(aExecute->m_noMutations);
			break;

		case EXECUTE_TYPE_MEDIAN_FILTER:
			MedianFilter(aExecute->m_value);
			break;

		case EXECUTE_TYPE_GROW:
			Grow(aExecute->m_value);
			break;

		case EXECUTE_TYPE_DESPECKLE:
			Despeckle();
			break;

		case EXECUTE_TYPE_ADD_BORDERS:
			AddBorders(aExecute->m_value);
			break;

		case EXECUTE_TYPE_DEBUG_TERRAIN_MAP:
			{
				TP_CHECK(m_terrainMap.size() > 0 && m_width > 0 && m_height > 0, "No terrain map.");
				Image image;
				image.Allocate(m_width, m_height);
				Image::RGBA* out = image.GetData();
				for(uint32_t i = 0; i < m_width * m_height; i++)
				{
					uint32_t terrainId = m_terrainMap[i];
					if(terrainId != 0)
					{
						const Data::Terrain* terrain = m_manifest->GetById<Data::Terrain>(terrainId);
						*out = terrain->m_debugColor;
					}
					else
					{
						*out = { 0, 0, 0, 255 };
					}
					out++;
				}
				image.SavePNG(aExecute->m_string.c_str());
			}
			break;

		case EXECUTE_TYPE_OVERLAY:
			ApplyOverlay(aExecute->m_overlay.get());
			break;

		case EXECUTE_TYPE_TERRAIN_MODIFIER_MAP:
			BuildTerrainModifierMap(aExecute->m_terrainModifierMap.get());
			break;

		default:
			TP_CHECK(false, "Invalid execute type.");
			break;
		}
	}

	uint32_t	
	World::Builder::GetTerrainPaletteEntry(
		char							aSymbol) const
	{
		TerrainPalette::const_iterator i = m_terrainPalette->find(aSymbol);
		TP_CHECK(i != m_terrainPalette->cend(), "'%c' not defined in terrain palette.");
		return i->second;
	}

	bool		
	World::Builder::MaybeDoIt(
		uint32_t						aProbabilty)
	{
		if(aProbabilty == 0 || aProbabilty == 100)
			return true;

		std::uniform_int_distribution<uint32_t> distribution(1, aProbabilty);
		return distribution(m_random) <= aProbabilty;
	}

	uint32_t	
	World::Builder::Roll(
		uint32_t						aMin,
		uint32_t						aMax)
	{
		if(aMax <= aMin)
			return aMin;

		std::uniform_int_distribution distribution(aMin, aMax);
		return distribution(m_random);
	}

	bool		
	World::Builder::RandomBit()
	{
		return m_random() & 0x8000000;
	}

	uint32_t	
	World::Builder::Random2Bits()
	{
		return m_random() >> 30;
	}

	uint32_t	
	World::Builder::GetTerrainIdWithOffset(
		uint32_t						aX,
		uint32_t						aY,
		int32_t							aOffsetX,
		int32_t							aOffsetY) const
	{
		int32_t x = (int32_t)aX + aOffsetX;
		int32_t y = (int32_t)aY + aOffsetY;
		if(x >= 0 && y >= 0 && x < (int32_t)m_width && y < (int32_t)m_height)
			return m_terrainMap[x + y * (int32_t)m_width];
		return 0;
	}

	void		
	World::Builder::Double(
		bool						aNoMutations)
	{
		TP_CHECK(m_terrainMap.size() > 0 && m_width > 0 && m_height > 0, "No terrain map.");

		uint32_t newWidth = m_width * 2;
		uint32_t newHeight = m_height * 2;
		std::vector<uint32_t> newTerrainMap;
		newTerrainMap.resize(newWidth * newHeight, 0);

		std::vector<Vec2> holes;

		for (int32_t y = 0; y < (int32_t)m_height; y++)
		{
			for (int32_t x = 0; x < (int32_t)m_width; x++)
			{
				static const Vec2 OFFSETS[4] = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } };
				uint32_t roll = Random2Bits();
				for(uint32_t i = 0; i < 4; i++)
				{
					const Vec2& offset = OFFSETS[i];
					Vec2 p = { 2 * x + offset.m_x, 2 * y + offset.m_y };
					if(i == roll)
						newTerrainMap[p.m_x + p.m_y * (int32_t)newWidth] = m_terrainMap[x + y * m_width];
					else
						holes.push_back(p);
				}
			}
		}

		while(holes.size() > 0)
		{
			std::vector<std::pair<int32_t, uint32_t>> changes;

			for(size_t i = 0; i < holes.size(); i++)
			{
				const Vec2& hole = holes[i];

				uint32_t candidates[4];
				uint32_t numCandidates = 0;

				static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
				for(uint32_t j = 0; j < 4; j++)
				{
					Vec2 p = { hole.m_x + NEIGHBORS[j].m_x, hole.m_y + NEIGHBORS[j].m_y };
					if(p.m_x >= 0 && p.m_y >= 0 && p.m_x < (int32_t)newWidth && p.m_y < (int32_t)newHeight)
					{
						uint32_t terrainId = newTerrainMap[p.m_x + p.m_y * (int32_t)newWidth];
						if(terrainId != 0)
							candidates[numCandidates++] = terrainId;
					}
				}

				if(numCandidates > 0)
				{
					changes.push_back({ hole.m_x + hole.m_y * (int32_t)newWidth, candidates[Roll(0, numCandidates - 1)] });

					Helpers::RemoveCyclicFromVector(holes, i);
					i--;
				}
			}

			assert(changes.size() > 0);

			for(const std::pair<int32_t, uint32_t>& change : changes)
				newTerrainMap[change.first] = change.second;
		}

		if(!aNoMutations)
		{
			for (uint32_t y = 1; y < newHeight - 1; y++)
			{
				for (uint32_t x = 1; x < newWidth - 1; x++)
				{
					uint32_t i = x + y * newWidth;
					const Data::Terrain* terrain = m_manifest->GetById<Data::Terrain>(newTerrainMap[i]);
					newTerrainMap[i] = terrain->Mutate(m_random);
				}
			}
		}

		m_terrainMap = std::move(newTerrainMap);
		m_width = newWidth;
		m_height = newHeight;
	}

	void		
	World::Builder::MedianFilter(
		uint32_t					aRadius)
	{
		typedef std::map<uint32_t, uint32_t> Counters;
		Counters counters;

		std::vector<uint32_t> newTerrainMap;
		newTerrainMap.resize(m_terrainMap.size());

		int32_t radiusSquared = (int32_t)(aRadius * aRadius);

		for (int32_t y = 0; y < (int32_t)m_height; y++)
		{
			for (int32_t x = 0; x < (int32_t)m_width; x++)
			{
				counters.clear();

				int32_t minX = Base::Max(x - (int32_t)aRadius, 0);
				int32_t minY = Base::Max(y - (int32_t)aRadius, 0);
				int32_t maxX = Base::Min(x + (int32_t)aRadius, (int32_t)m_width - 1);
				int32_t maxY = Base::Min(y + (int32_t)aRadius, (int32_t)m_height - 1);
				
				for(int32_t iy = minY; iy <= maxY; iy++)
				{
					for (int32_t ix = minX; ix <= maxX; ix++)
					{
						Vec2 d = { ix - x, iy - y };
						if(d.m_x * d.m_x + d.m_y * d.m_y <= radiusSquared)
						{
							uint32_t terrainId = m_terrainMap[ix + iy * (int32_t)m_width];

							Counters::iterator i = counters.find(terrainId);
							if (i == counters.end())
								counters[terrainId] = 1;
							else
								i->second++;
						}
					}
				}

				assert(counters.size() > 0);
				newTerrainMap[x + y * (int32_t)m_width] = counters.cbegin()->first;
			}
		}

		m_terrainMap = std::move(newTerrainMap);
	}

	void		
	World::Builder::Grow(
		uint32_t					aTerrainId)
	{
		const Data::Terrain* terrain = m_manifest->GetById<Data::Terrain>(aTerrainId);

		std::unordered_map<uint32_t, uint32_t> probabilites;
		for(const Data::Terrain::GrowsInto& growsInto : terrain->m_growsInto)
			probabilites[growsInto.m_terrainId] = growsInto.m_probability;

		std::vector<uint32_t> changes;

		for (uint32_t y = 0; y < m_height; y++)
		{
			for (uint32_t x = 0; x < m_width; x++)
			{
				uint32_t offset = x + y * m_width;
				std::unordered_map<uint32_t, uint32_t>::const_iterator i = probabilites.find(m_terrainMap[offset]);
				if(i != probabilites.cend())
				{
					uint32_t probability = i->second;
					static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
					for (uint32_t j = 0; j < 4; j++)
					{
						const Vec2& neighbor = NEIGHBORS[j];
						if(GetTerrainIdWithOffset(x, y, neighbor.m_x, neighbor.m_y) == aTerrainId)
						{
							if (Roll(1, 100) <= probability)
							{
								changes.push_back(offset);
								break;
							}
						}
					}
				}
			}
		}

		for(uint32_t change : changes)
			m_terrainMap[change] = aTerrainId;
	}

	void		
	World::Builder::Despeckle()
	{
		std::vector<std::pair<uint32_t, uint32_t>> changes;

		for (uint32_t y = 0; y < m_height; y++)
		{
			for (uint32_t x = 0; x < m_width; x++)
			{
				uint32_t offset = x + y * m_width;
				uint32_t terrainId = m_terrainMap[offset];
				uint32_t surroundingTerrainId = 0;

				static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
				for (uint32_t j = 0; j < 4; j++)
				{
					const Vec2& neighbor = NEIGHBORS[j];
					uint32_t neighborTerrainId = GetTerrainIdWithOffset(x, y, neighbor.m_x, neighbor.m_y);
					if(neighborTerrainId == terrainId || (surroundingTerrainId != 0 && surroundingTerrainId != neighborTerrainId))
					{
						surroundingTerrainId = 0;
						break;
					}
					else if(surroundingTerrainId == 0)
					{
						surroundingTerrainId = neighborTerrainId;
					}
				}

				if(surroundingTerrainId != 0)
				{
					changes.push_back({ offset, surroundingTerrainId });
				}
			}
		}

		for (const std::pair<uint32_t, uint32_t>& change : changes)
			m_terrainMap[change.first] = change.second;
	}

	void		
	World::Builder::AddBorders(
		uint32_t					aTerrainId)
	{
		const Data::Terrain* terrain = m_manifest->GetById<Data::Terrain>(aTerrainId);
		
		std::unordered_map<uint32_t, uint32_t> borders;
		for(const Data::Terrain::Border& border : terrain->m_borders)
		{
			for(uint32_t neighborTerrainId : border.m_neighborTerrain)
				borders[neighborTerrainId] = border.m_terrainId;
		}

		std::vector<std::pair<uint32_t, uint32_t>> changes;

		for (uint32_t y = 0; y < m_height; y++)
		{
			for (uint32_t x = 0; x < m_width; x++)
			{
				uint32_t offset = x + y * m_width;
				if(m_terrainMap[offset] == aTerrainId)
				{
					uint32_t candidates[4];
					uint32_t numCandidates = 0;

					static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
					for (uint32_t j = 0; j < 4; j++)
					{
						const Vec2& neighbor = NEIGHBORS[j];
						uint32_t neighborTerrainId = GetTerrainIdWithOffset(x, y, neighbor.m_x, neighbor.m_y);
						std::unordered_map<uint32_t, uint32_t>::const_iterator i = borders.find(neighborTerrainId);
						if (i != borders.cend())
							candidates[numCandidates++] = i->second;
					}

					if (numCandidates > 0)
						changes.push_back({ offset, candidates[Roll(0, numCandidates - 1)] });
				}
			}
		}

		for (const std::pair<uint32_t, uint32_t>& change : changes)
			m_terrainMap[change.first] = change.second;
	}

	void		
	World::Builder::ApplyOverlay(
		const Overlay*				aOverlay)
	{
		const Data::Noise* noise = m_manifest->GetById<Data::Noise>(aOverlay->m_noiseId);
		NoiseInstance noiseInstance(noise, m_random);

		uint32_t offset = 0;
		for(int32_t y = 0; y < (int32_t)m_height; y++)
		{
			for (int32_t x = 0; x < (int32_t)m_width; x++)
			{
				int32_t sample = noiseInstance.Sample({ x, y });
				uint32_t terrainId = m_terrainMap[offset];

				uint32_t newTerrainId = 0;

				for(const std::unique_ptr<OverlayTerrain>& overlayTerrain : aOverlay->m_terrains)
				{
					if(overlayTerrain->m_condition.Check(sample) && !overlayTerrain->IsExcluded(terrainId))
					{
						newTerrainId = overlayTerrain->m_terrainId;
						break;
					}
				}

				if(newTerrainId != 0)
					m_terrainMap[offset] = newTerrainId;

				offset++;
			}
		}
	}

	void		
	World::Builder::BuildTerrainModifierMap(
		const TerrainModifierMap*	aTerrainModifierMap)
	{
		TP_CHECK(TerrainModifier::ValidateId(aTerrainModifierMap->m_id), "No terrain modifier specified.");
		std::vector<int32_t>& terrainModifierMap = m_terrainModifierMaps[aTerrainModifierMap->m_id];
		terrainModifierMap.resize(m_width * m_height, 0);

		const Data::Noise* noise = m_manifest->GetById<Data::Noise>(aTerrainModifierMap->m_noiseId);
		NoiseInstance noiseInstance(noise, m_random);

		uint32_t offset = 0;
		for (int32_t y = 0; y < (int32_t)m_height; y++)
		{
			for (int32_t x = 0; x < (int32_t)m_width; x++)
			{
				int32_t sample = noiseInstance.Sample({ x, y });

				terrainModifierMap[offset] = sample;

				offset++;
			}
		}

		if(!aTerrainModifierMap->m_debug.empty())
		{
			Image image;
			image.Allocate(m_width, m_height);

			Image::RGBA* out = image.GetData();

			for (uint32_t i = 0; i < m_width * m_height; i++)
			{
				int32_t v = terrainModifierMap[i];
				if(v < 0)
					v = 0;
				else if(v > 255)
					v = 255;

				out->m_r = (uint8_t)v;
				out->m_g = (uint8_t)v;
				out->m_b = (uint8_t)v;
				out->m_a = 255;
				out++;
			}

			image.SavePNG(aTerrainModifierMap->m_debug.c_str());
		}
	}

	void		
	World::Builder::CreateMapData(
		const MapData*				aSourceMapData,
		std::unique_ptr<MapData>&	aOutMapData)
	{
		aOutMapData = std::make_unique<MapData>();
		aOutMapData->m_mapInfo = aSourceMapData->m_mapInfo;

		// Create blank tile map
		{
			aOutMapData->m_width = (int32_t)m_width;
			aOutMapData->m_height = (int32_t)m_height;

			aOutMapData->m_tileMap = new uint32_t[m_width * m_height];
			for (uint32_t i = 0; i < m_width * m_height; i++)
				aOutMapData->m_tileMap[i] = aOutMapData->m_mapInfo.m_defaultTileSpriteId;
		}

		// Create tiles
		for (uint32_t i = 0, count = m_width * m_height; i < count; i++)
		{
			uint32_t terrainId = m_terrainMap[i];
			const Data::Terrain* terrain = m_manifest->GetById<Data::Terrain>(terrainId);

			uint32_t tileSpriteId = 0;

			for(const std::unique_ptr<Data::Terrain::Tile>& tile : terrain->m_tiles)
			{
				bool ok = true;

				for(const Data::Terrain::TileThreshold& tileThreshold : tile->m_thresholds)
				{
					TP_CHECK(TerrainModifier::ValidateId(tileThreshold.m_terrainModifierId), "Invalid terrain modifier.");
					std::vector<int32_t>& terrainModifierMap = m_terrainModifierMaps[tileThreshold.m_terrainModifierId];
					if(!terrainModifierMap.empty() && !tileThreshold.Check(terrainModifierMap[i]))
					{
						ok = false;
						break;
					}
				}

				if(ok)
				{
					tileSpriteId = tile->m_tileSpriteId;
					break;
				}
			}
			
			if(tileSpriteId != 0)
				aOutMapData->m_tileMap[i] = tileSpriteId;
		}

		// Player spawns
		{
			MapData::PlayerSpawn playerSpawn;
			playerSpawn.m_id = aOutMapData->m_mapInfo.m_defaultPlayerSpawnId;
			playerSpawn.m_x = m_walkable.cbegin()->m_x;
			playerSpawn.m_y = m_walkable.cbegin()->m_y;
			aOutMapData->m_playerSpawns.push_back(playerSpawn);			
		}
	}

	void		
	World::Builder::IdentifyWalkableAreas()
	{
		std::unordered_set<Vec2, Vec2::Hasher> positions;

		for (int32_t y = 0; y < (int32_t)m_height; y++)
		{
			for (int32_t x = 0; x < (int32_t)m_width; x++)
			{
				uint32_t terrainId = m_terrainMap[x + y * (int32_t)m_width];
				const Data::Terrain* terrain = m_manifest->GetById<Data::Terrain>(terrainId);

				if (terrain->m_walkable)
					positions.insert({ x, y });
			}
		}

		while(positions.size() > 0)
		{
			// Grab walkable position not assigned to an area yet
			Vec2 startPosition = *positions.begin();

			// Flood-fill from there
			std::unordered_set<Vec2, Vec2::Hasher> queue;
			queue.insert(startPosition);

			WalkableArea* walkableArea = new WalkableArea();
			m_walkableAreas.push_back(std::unique_ptr<WalkableArea>(walkableArea));

			while(queue.size() > 0)
			{
				Vec2 position = *queue.begin();
				queue.erase(position);
				positions.erase(position);
				walkableArea->m_positions.insert(position);

				static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
				for(size_t i = 0; i < 4; i++)
				{
					Vec2 nextPosition = { position.m_x + NEIGHBORS[i].m_x, position.m_y + NEIGHBORS[i].m_y };
					if(positions.contains(nextPosition))
						queue.insert(nextPosition);
				}
			}
		}

		std::sort(m_walkableAreas.begin(), m_walkableAreas.end(), [](
			const std::unique_ptr<WalkableArea>& aLHS,
			const std::unique_ptr<WalkableArea>& aRHS)
		{
			return aLHS->m_positions.size() < aRHS->m_positions.size();
		});
	}

	void		
	World::Builder::ConnectWalkableAreas()
	{
		DistanceField distanceField((int32_t)m_width, (int32_t)m_height);

		while(TryConnectWalkableArea(distanceField))
		{
			// Keep calling until it returns false (all walkable areas completed)
		}
	
		// Grab largest walkable area and forget about the rest
		WalkableArea* walkableArea = m_walkableAreas[0].get();

		for (size_t i = 1; i < m_walkableAreas.size(); i++)
		{
			if (m_walkableAreas[i]->m_positions.size() > walkableArea->m_positions.size())
				walkableArea = m_walkableAreas[i].get();
		}

		m_walkable = walkableArea->m_positions;
		m_walkableAreas.clear();
	}

	bool
	World::Builder::TryConnectWalkableArea(
		DistanceField&		aDistanceField)
	{
		// Get first incomplete walkable area
		WalkableArea* walkableArea = NULL;
		size_t walkableAreaIndex = SIZE_MAX;

		for(size_t i = 0; i < m_walkableAreas.size(); i++)
		{
			if(!m_walkableAreas[i]->m_complete)
			{
				walkableArea = m_walkableAreas[i].get();
				walkableAreaIndex = i;
			}
		}

		if(walkableArea == NULL)
			return false;

		typedef std::multimap<uint32_t, Vec2> Queue;
		Queue queue;

		aDistanceField.Clear();

		// Initialize queue with walkable area edge
		{
			std::unordered_set<Vec2, Vec2::Hasher> edge;

			for (const Vec2& position : walkableArea->m_positions)
			{
				aDistanceField.Set(position, 0);

				static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
				for (size_t j = 0; j < 4; j++)
				{
					Vec2 p = { position.m_x + NEIGHBORS[j].m_x, position.m_y + NEIGHBORS[j].m_y };
					if (p.m_x >= 0 && p.m_y >= 0 && p.m_x < (int32_t)m_width && p.m_y < (int32_t)m_height)
					{
						if (!walkableArea->m_positions.contains(p))
							edge.insert(p);
					}
				}
			}

			for(const Vec2& p : edge)
				queue.insert({ 0, p });
		}

		// Find what position (of other area) this walkable area can be connected to
		assert(queue.size() > 0);
		std::optional<Vec2> connectPosition;

		while(queue.size() > 0 && !connectPosition.has_value())
		{
			Queue::const_iterator front = queue.cbegin();
			uint32_t previousDistance = front->first;
			Vec2 position = front->second;
			queue.erase(front);

			uint32_t terrainId = m_terrainMap[position.m_x + position.m_y * (int32_t)m_width];
			const Data::Terrain* terrain = m_manifest->GetById<Data::Terrain>(terrainId);

			// 2D noise is added to the distance field to prevent paths from being too straight
			uint32_t distance = previousDistance + terrain->m_connectCost + SampleNoiseMap(position);
			uint32_t& distanceMapReference = aDistanceField.GetReference(position);

			if (distance < distanceMapReference && distance < m_maxWalkableAreaConnectDistance)
			{
				distanceMapReference = distance;

				if(terrain->m_walkable)
				{
					connectPosition = position;
				}
				else
				{
					static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
					for (size_t j = 0; j < 4; j++)
					{
						Vec2 p = { position.m_x + NEIGHBORS[j].m_x, position.m_y + NEIGHBORS[j].m_y };
						if (p.m_x >= 0 && p.m_y >= 0 && p.m_x < (int32_t)m_width && p.m_y < (int32_t)m_height)
							queue.insert({ distance, p });
					}
				}
			}
		}

		if(connectPosition.has_value())
		{
			// We got a connection - make a path
			std::vector<Vec2> path;
			bool ok = aDistanceField.MakePath(m_random, connectPosition.value(), path);
			assert(ok);

			size_t otherWalkableAreaIndex = GetWalkableAreaIndexByPosition(connectPosition.value());
			assert(otherWalkableAreaIndex != SIZE_MAX);
			assert(otherWalkableAreaIndex != walkableAreaIndex);
			WalkableArea* otherWalkableArea = m_walkableAreas[otherWalkableAreaIndex].get();

			otherWalkableArea->Merge(walkableArea);

			for (size_t j = 0; j < path.size(); j++)
			{
				const Vec2& pathPosition = path[j];
				otherWalkableArea->m_positions.insert(pathPosition);

				uint32_t terrainId = m_terrainMap[pathPosition.m_x + pathPosition.m_y * (int32_t)m_width];
				const Data::Terrain* terrain = m_manifest->GetById<Data::Terrain>(terrainId);
				if(!terrain->m_walkable)
				{
					TP_CHECK(terrain->m_connectConversion != 0, "Missing connect conversion: %s", terrain->m_name.c_str());
					m_terrainMap[pathPosition.m_x + pathPosition.m_y * (int32_t)m_width] = terrain->m_connectConversion;
				}
			}

			m_walkableAreas.erase(m_walkableAreas.begin() + walkableAreaIndex);
		}
		else
		{
			walkableArea->m_complete = true;
		}
				
		return true;
	}

	size_t
	World::Builder::GetWalkableAreaIndexByPosition(
		const Vec2&			aPosition)
	{		
		for(size_t i = 0; i < m_walkableAreas.size(); i++)
		{
			if(m_walkableAreas[i]->m_positions.contains(aPosition))
				return i;
		}
		return SIZE_MAX;
	}

	void			
	World::Builder::InitNoiseMap()
	{
		uint32_t* p = m_noiseMap;
		for(int32_t i = 0; i < NOISE_MAP_SIZE * NOISE_MAP_SIZE; i++)
		{
			uint32_t n = m_random();
			if (n < UINT32_MAX / 11)
				*p = 4;
			else if (n < UINT32_MAX / 5)
				*p = 3;
			else if (n < UINT32_MAX / 4)
				*p = 2;
			else if (n < UINT32_MAX / 3)
				*p = 1;
			else
				*p = 0;
			p++;
		}
	}

	uint32_t		
	World::Builder::SampleNoiseMap(
		const Vec2&			aPosition) const
	{
		assert(aPosition.m_x >= 0 && aPosition.m_y >= 0 && aPosition.m_x < (int32_t)m_width && aPosition.m_y < (int32_t)m_height);
		Vec2 p = { aPosition.m_x % NOISE_MAP_SIZE, aPosition.m_y % NOISE_MAP_SIZE };
		return m_noiseMap[p.m_x + p.m_y * NOISE_MAP_SIZE];
	}

}

