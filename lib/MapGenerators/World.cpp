#include "../Pcheader.h"

#include <tpublic/Components/CombatPublic.h>

#include <tpublic/Data/Doodad.h>
#include <tpublic/Data/Faction.h>
#include <tpublic/Data/MapSegment.h>
#include <tpublic/Data/MapEntitySpawn.h>
#include <tpublic/Data/Noise.h>
#include <tpublic/Data/Sprite.h>
#include <tpublic/Data/TagContext.h>
#include <tpublic/Data/Terrain.h>

#include <tpublic/MapGenerators/World.h>

#include <tpublic/DistanceField.h>
#include <tpublic/DoodadPlacement.h>
#include <tpublic/Helpers.h>
#include <tpublic/Image.h>
#include <tpublic/Manifest.h>
#include <tpublic/MapGeneratorRuntime.h>
#include <tpublic/MapRouteData.h>
#include <tpublic/NoiseInstance.h>
#include <tpublic/TaggedData.h>
#include <tpublic/WorldInfoMap.h>

namespace tpublic::MapGenerators
{

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

		UniformDistribution<uint32_t> distribution(1, aProbabilty);
		return distribution(m_random) <= aProbabilty;
	}

	uint32_t	
	World::Builder::Roll(
		uint32_t						aMin,
		uint32_t						aMax)
	{
		if(aMax <= aMin)
			return aMin;

		UniformDistribution distribution(aMin, aMax);
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
			uint32_t tileSpriteId = GetTerrainTileSpriteId(m_terrainMap[i], (size_t)i);
			if(tileSpriteId != 0)
				aOutMapData->m_tileMap[i] = tileSpriteId;
		}

		// Structures
		std::vector<uint32_t> coverMap;

		for(const Structure& structure : m_structures)
		{
			const Data::MapSegment::TileMap* tileMap = structure.m_mapSegment->m_tileMap.get();

			size_t i = 0;
			for(int32_t y = 0; y < tileMap->m_size.m_y; y++)
			{
				for(int32_t x = 0; x < tileMap->m_size.m_x; x++)
				{
					size_t mapIndex = (size_t)(structure.m_position.m_x + x) + (size_t)(structure.m_position.m_y + y) * (size_t)m_width;

					if(tileMap->m_tiles[i] != 0)
						aOutMapData->m_tileMap[mapIndex] = tileMap->m_tiles[i];

					if(!tileMap->m_coverTiles.empty() && tileMap->m_coverTiles[i] != 0)
					{
						if(coverMap.empty())
							coverMap.resize((size_t)(m_width * m_height), 0);

						coverMap[mapIndex] = tileMap->m_coverTiles[i];
					}

					i++;
				}
			}

			for(const Data::MapSegment::Object& object : tileMap->m_objects)
			{
				switch(object.m_type)
				{
				case Data::MapSegment::OBJECT_TYPE_ENTITY_SPAWN:
					{
						MapData::EntitySpawn t;
						t.m_mapEntitySpawnId = object.m_id;
						t.m_x = structure.m_position.m_x + object.m_position.m_x;
						t.m_y = structure.m_position.m_y + object.m_position.m_y;
						aOutMapData->m_entitySpawns.push_back(t);
					}
					break;

				case Data::MapSegment::OBJECT_TYPE_PORTAL:
					{
						MapData::Portal t;
						t.m_id = object.m_id;
						t.m_x = structure.m_position.m_x + object.m_position.m_x;
						t.m_y = structure.m_position.m_y + object.m_position.m_y;
						aOutMapData->m_portals.push_back(t);
					}
					break;

				default:
					TP_CHECK(false, "Map segment '%s' has objects that can't be used in a generated world.", structure.m_mapSegment->m_name.c_str());
					break;
				}
			}

			for(const MapData::EntitySpawn& entitySpawn : structure.m_anchorEntitySpawns)
				aOutMapData->m_entitySpawns.push_back(entitySpawn);
		}

		// Caves are covered by the tiles of what was dug out, so they look the same from the outside
		for(size_t i = 0; i < m_caveCoverTerrainMap.size(); i++)
		{
			if(m_caveCoverTerrainMap[i] == 0)
				continue;

			uint32_t tileSpriteId = GetTerrainTileSpriteId(m_caveCoverTerrainMap[i], i);
			if(tileSpriteId == 0)
				continue;

			if(coverMap.empty())
				coverMap.resize((size_t)(m_width * m_height), 0);

			coverMap[i] = tileSpriteId;
		}

		for(const std::pair<size_t, uint32_t>& entranceCover : m_caveEntranceCovers)
		{
			if(coverMap.empty())
				coverMap.resize((size_t)(m_width * m_height), 0);

			coverMap[entranceCover.first] = entranceCover.second;
		}

		// Player spawns
		for (const std::unique_ptr<PlayerSpawn>& playerSpawn : m_playerSpawns)
		{
			MapData::PlayerSpawn t;
			t.m_id = aOutMapData->m_mapInfo.m_defaultPlayerSpawnId;
			t.m_x = playerSpawn->m_position.m_x;
			t.m_y = playerSpawn->m_position.m_y;
			aOutMapData->m_playerSpawns.push_back(t);		
			
			MapData::EntitySpawn object;
			object.m_mapEntitySpawnId = m_params->m_objectMapEntitySpawnId;
			object.m_entityId = m_params->m_playerSpawnEntityId;
			object.m_x = t.m_x;
			object.m_y = t.m_y;
			aOutMapData->m_entitySpawns.push_back(object);
		}

		// Boss entity spawns
		for(const std::unique_ptr<Boss>& boss : m_bosses)
		{
			{
				MapData::EntitySpawn entitySpawn;
				entitySpawn.m_entityId = boss->m_entity->m_id;
				entitySpawn.m_mapEntitySpawnId = boss->m_mapEntitySpawnId;
				entitySpawn.m_x = boss->m_position.m_x;
				entitySpawn.m_y = boss->m_position.m_y;
				aOutMapData->m_entitySpawns.push_back(entitySpawn);
			}

			// Sub-bosses
			for(const MinorBoss& minorBoss : boss->m_subBosses)
			{
				MapData::EntitySpawn entitySpawn;
				entitySpawn.m_entityId = minorBoss.m_entity->m_id;
				entitySpawn.m_mapEntitySpawnId = boss->m_minorBosses->m_mapEntitySpawnId;
				entitySpawn.m_x = minorBoss.m_position.m_x;
				entitySpawn.m_y = minorBoss.m_position.m_y;
				aOutMapData->m_entitySpawns.push_back(entitySpawn);
			}
		}

		// More entity spawns
		for (const MapData::EntitySpawn& entitySpawn : m_entitySpawns)
			aOutMapData->m_entitySpawns.push_back(entitySpawn);

		// World info map
		{
			std::vector<uint32_t> levels;
			levels.resize(m_width * m_height);
			for(uint32_t i = 0, count = m_width * m_height; i < count; i++)
				levels[i] = m_levelMap[i].m_level;

			const uint32_t* zoneMap = m_zoneMap.empty() ? NULL : &m_zoneMap[0];

			std::vector<uint8_t> flags;
			if(!coverMap.empty() && aOutMapData->m_mapInfo.m_autoIndoor)
			{
				flags.resize((size_t)(m_width * m_height), 0);
				WorldInfoMap::AutoIndoor(m_manifest, (int32_t)m_width, (int32_t)m_height, &coverMap[0], &flags[0]);
			}

			aOutMapData->m_worldInfoMap = std::make_unique<WorldInfoMap>();
			aOutMapData->m_worldInfoMap->Build(m_manifest, (int32_t)m_width, (int32_t)m_height, &levels[0], NULL, zoneMap, flags.empty() ? NULL : &flags[0]);
		}

		// Roofs and other covers from structures
		if(!coverMap.empty())
		{
			aOutMapData->m_mapCovers = std::make_unique<MapCovers>();
			aOutMapData->m_mapCovers->Build((int32_t)m_width, (int32_t)m_height, &coverMap[0]);
		}

		// Doodads
		{
			std::unordered_map<Vec2, uint32_t, Vec2::Hasher> coverageMap;

			if(!m_structureMap.empty())
			{
				for(int32_t y = 0; y < (int32_t)m_height; y++)
				{
					for(int32_t x = 0; x < (int32_t)m_width; x++)
					{
						if(m_structureMap[x + y * (int32_t)m_width] != STRUCTURE_MAP_NONE)
							coverageMap[{ x, y }] = 1;
					}
				}
			}

			// Doodads that are part of structures go first, without placement checks
			for(const Structure& structure : m_structures)
			{
				for(const Data::MapSegment::TileMap::DoodadObject& structureDoodad : structure.m_mapSegment->m_tileMap->m_doodads)
				{
					const Data::Doodad* doodadData = m_manifest->GetById<Data::Doodad>(structureDoodad.m_doodadId);
					if(doodadData->m_spriteIds.empty())
						continue;

					Vec2 position = { structure.m_position.m_x + structureDoodad.m_position.m_x, structure.m_position.m_y + structureDoodad.m_position.m_y };
					tpublic::UniformDistribution<size_t> distribution(0, doodadData->m_spriteIds.size() - 1);
					aOutMapData->m_doodads[position] = doodadData->m_spriteIds[distribution(m_random)];

					DoodadPlacement::AddToCoverageMap(doodadData, position, coverageMap);
				}
			}

			for (const Doodad& doodad : m_doodads)
			{
				Vec2 mapSize = { (int32_t)m_width, (int32_t)m_height };
				const Data::Doodad* doodadData = m_manifest->GetById<Data::Doodad>(doodad.m_doodadId);
				if (doodadData->m_spriteIds.size() > 0 && DoodadPlacement::Check(aOutMapData->m_tileMap, mapSize, doodadData, doodad.m_position, coverageMap))
				{
					tpublic::UniformDistribution<size_t> distribution(0, doodadData->m_spriteIds.size() - 1);
					aOutMapData->m_doodads[doodad.m_position] = doodadData->m_spriteIds[distribution(m_random)];

					DoodadPlacement::AddToCoverageMap(doodadData, doodad.m_position, coverageMap);
				}
			}
		}

		// Routes 
		{
			for(const std::unique_ptr<Route>& route : m_routes)
			{
				if(!aOutMapData->m_mapRouteData)
					aOutMapData->m_mapRouteData = std::make_unique<MapRouteData>();

				MapRouteData::Route* routeData = aOutMapData->m_mapRouteData->GetOrCreateRoute(route->m_routeId);
				for(const Vec2& position : route->m_positions)
					routeData->m_positions.insert(position);
			}

			if(aOutMapData->m_mapRouteData)
			{
				//aOutMapData->m_mapRouteData->Build(
				//	m_manifest,
				//	aOutMapData->m_tileMap, 
				//	(int32_t)aOutMapData->m_width,
				//	(int32_t)aOutMapData->m_height,
				//	aW
			}
		}
	}

	void		
	World::Builder::IdentifyWalkableAreas()
	{
		std::set<Vec2> positions;

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

		uint32_t nextWalkableAreaId = 0;

		while(positions.size() > 0)
		{
			// Grab walkable position not assigned to an area yet
			Vec2 startPosition = *positions.begin();

			// Flood-fill from there
			std::set<Vec2> queue;
			queue.insert(startPosition);

			WalkableArea* walkableArea = new WalkableArea();
			walkableArea->m_id = nextWalkableAreaId++;
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
			if(aLHS->m_positions.size() == aRHS->m_positions.size())
				return aLHS->m_id < aRHS->m_id;
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
			std::set<Vec2> edge;

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
			(void)ok;
			assert(ok);

			size_t otherWalkableAreaIndex = GetWalkableAreaIndexByPosition(connectPosition.value());
			assert(otherWalkableAreaIndex != SIZE_MAX);
			assert(otherWalkableAreaIndex != walkableAreaIndex);
			WalkableArea* otherWalkableArea = m_walkableAreas[otherWalkableAreaIndex].get();

			otherWalkableArea->Merge(walkableArea);

			std::set<Vec2> pathPositions;

			for (size_t j = 0; j < path.size(); j++)
			{
				const Vec2& pathPosition = path[j];

				int32_t radius = 0;
				if(!m_params->m_connectConversionRadius.empty())
					radius = (int32_t)Helpers::RandomItem(m_random, m_params->m_connectConversionRadius);

				for(int32_t y = -radius; y <= radius; y++)
				{
					for (int32_t x = -radius; x <= radius; x++)
					{
						if(x * x + y * y <= radius * radius)
							pathPositions.insert({ pathPosition.m_x + x, pathPosition.m_y + y });
					}
				}
			}

			for(const Vec2& pathPosition : pathPositions)
			{
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

	void			
	World::Builder::UpdateZoneMap(
		const Vec2&					aPosition,
		uint32_t					aZoneId)
	{
		if(aPosition.m_x < 0 || aPosition.m_y < 0 || aPosition.m_x >= (int32_t)m_width || aPosition.m_y >= (int32_t)m_height)
			return;

		if(m_zoneMap.empty())
			m_zoneMap.resize(m_width * m_height);

		m_zoneMap[aPosition.m_x + aPosition.m_y * (int32_t)m_width] = aZoneId;
	}

	void			
	World::Builder::InitBosses()
	{
		for(std::unique_ptr<Boss>& boss : m_bosses)
		{
			Vec2 position;

			if(!m_bossDistanceCombined)
			{
				// First boss is placed randomly
				position = *m_walkable.cbegin();
			}
			else
			{
				// Remaining bosses are more complicated to place - put them as far away from other bosses as possible (or a lil bit closer otherwise it would look weird)
				std::vector<Vec2> positions;
				m_bossDistanceCombined->GetPositionsWithValue(m_bossDistanceCombined->GetMax() - 5, positions);
				if(positions.empty())
				{
					// Shouldn't really happen, just don't place any more bosses
					break;
				}

				position = positions[Roll(0, (uint32_t)positions.size() - 1)];
			}

			boss->m_position = position;
			boss->m_distanceField = std::make_unique<DistanceField>((int32_t)m_width, (int32_t)m_height);
			boss->m_distanceField->Generate({ position }, m_walkable, UINT32_MAX);

			m_entitySpawnPositions.insert(position);

			if(!m_bossDistanceCombined)
				m_bossDistanceCombined = std::make_unique<DistanceField>((int32_t)m_width, (int32_t)m_height);

			m_bossDistanceCombined->CombineMin(boss->m_distanceField.get());

			// Convert boss distance field into an "influence field"
			boss->m_distanceField->Filter([&](
				uint32_t aValue)
			{
				if(aValue > boss->m_influence)
					return UINT32_MAX;
				else
					return boss->m_influence - aValue;
			});

			// Pick boss 			
			{
				const TaggedData::QueryResult* result = m_mapGeneratorRuntime->m_entities->PerformQueryWithTagContext(m_manifest->GetById<Data::TagContext>(boss->m_tagContextId));
				const TaggedData::QueryResult::Entry* picked = result->TryPickRandomFiltered(m_random, [&](
					const TaggedData::QueryResult::Entry* aEntry) -> bool
				{
					const Data::Entity* entity = m_manifest->GetById<Data::Entity>(aEntry->m_id);
					const Components::CombatPublic* combatPublic = entity->TryGetComponent<Components::CombatPublic>();
					if(combatPublic->m_level == m_levelRange.m_max)
					{	
						boss->m_factionId = combatPublic->m_factionId;
						return true;
					}
					return false;
				});

				TP_CHECK(picked != NULL, "Unable to pick boss.");
				boss->m_entity = m_manifest->GetById<Data::Entity>(picked->m_id);
			}
		}

		// Make level map
		m_levelMap.resize(m_width * m_height);
		for(uint32_t i = 0, count = m_width * m_height; i < count; i++)
		{
			LevelMapPoint& levelMapPoint = m_levelMap[i];

			// Find boss with highest influence here
			levelMapPoint.m_boss = NULL;
			levelMapPoint.m_influence = UINT32_MAX;
			levelMapPoint.m_level = m_levelRange.m_min;

			for (const std::unique_ptr<Boss>& boss : m_bosses)
			{
				uint32_t bossInfluence = boss->m_distanceField->m_data[i];

				if(bossInfluence != UINT32_MAX && (levelMapPoint.m_boss == NULL || bossInfluence > levelMapPoint.m_influence))
				{
					levelMapPoint.m_boss = boss.get();
					levelMapPoint.m_influence = bossInfluence;
				}
			}

			if(levelMapPoint.m_boss != NULL)
			{
				// Calculate area level based boss influence
				levelMapPoint.m_level = m_levelRange.m_min + ((m_levelRange.m_max - m_levelRange.m_min + 1) * levelMapPoint.m_influence) / levelMapPoint.m_boss->m_influence;
			}
		}	

		// Minor bosses
		for (std::unique_ptr<Boss>& boss : m_bosses)
		{
			if(boss->m_minorBosses != NULL)
			{
				uint32_t count = boss->m_minorBosses->m_count.GetRandom(m_random);

				uint32_t influence = 0;
				uint32_t influenceStep = 0;

				if(count > 1)
				{
					// Spread out the minor bosses over the desired influence range
					influence = boss->m_minorBosses->m_influenceRange.m_max;
					influenceStep = (boss->m_minorBosses->m_influenceRange.m_max - boss->m_minorBosses->m_influenceRange.m_min) / (count - 1);
				}
				else if(count == 1)
				{
					// With only one minor boss we'll just put it somewhere random in the range
					influence = boss->m_minorBosses->m_influenceRange.GetRandom(m_random);
				}

				for(uint32_t i = 0; i < count; i++)
				{
					std::vector<Vec2> positions;
					boss->m_distanceField->GetPositionsWithValue(influence, positions);
					if(positions.size() > 0)
					{
						Vec2 position = positions[Roll(0, (uint32_t)positions.size() - 1)];
						const LevelMapPoint& levelMapPoint = m_levelMap[position.m_x + position.m_y * (int32_t)m_width];
						uint32_t minorBossLevel = levelMapPoint.m_level;

						// Pick minor boss 			
						const TaggedData::QueryResult* result = m_mapGeneratorRuntime->m_entities->PerformQueryWithTagContext(m_manifest->GetById<Data::TagContext>(boss->m_minorBosses->m_tagContextId));
						const TaggedData::QueryResult::Entry* picked = result->TryPickRandomFiltered(m_random, [&](
							const TaggedData::QueryResult::Entry* aEntry) -> bool
						{
							const Data::Entity* entity = m_manifest->GetById<Data::Entity>(aEntry->m_id);
							const Components::CombatPublic* combatPublic = entity->TryGetComponent<Components::CombatPublic>();
							if (combatPublic->m_level == minorBossLevel && combatPublic->m_factionId == boss->m_factionId)
								return true;
							return false;
						});

						TP_CHECK(picked != NULL, "Unable to pick minor boss.");
							
						MinorBoss minorBoss;
						minorBoss.m_entity = m_manifest->GetById<Data::Entity>(picked->m_id);
						minorBoss.m_position = position;
						boss->m_subBosses.push_back(minorBoss);

						m_entitySpawnPositions.insert(position);
					}

					influence -= influenceStep;
				}
			}
		}

		// Enemy packs
		for (int32_t y = 0; y < (int32_t)m_height; y++)
		{
			for (int32_t x = 0; x < (int32_t)m_width; x++)
			{
				const LevelMapPoint& point = m_levelMap[x + y * (int32_t)m_width];
				if (point.m_boss != NULL && !m_entitySpawnPositions.contains({ x, y }))
				{
					const Pack* pickedPack = NULL;

					for (const Pack* pack : point.m_boss->m_packs)
					{
						if (point.m_influence >= pack->m_influenceRange.m_min && point.m_influence <= pack->m_influenceRange.m_max)
						{
							if (Roll(1, 1000) <= pack->m_probability && CanPlacePack(pack, { x, y }))
							{
								pickedPack = pack;
								break;
							}
						}
					}

					if (pickedPack != NULL)
						PlacePack(pickedPack, { x, y }, point.m_level, point.m_boss->m_factionId);
				}
			}
		}
	}

	bool
	World::Builder::CanPlacePack(
		const Pack*						aPack,
		const Vec2&						aPosition) const
	{
		// Check if any other enemy is too close
		int32_t minDistanceSquared = (int32_t)(aPack->m_minDistanceToNearBy * aPack->m_minDistanceToNearBy);

		for (int32_t iy = aPosition.m_y - (int32_t)aPack->m_minDistanceToNearBy; iy <= aPosition.m_y + (int32_t)aPack->m_minDistanceToNearBy; iy++)
		{
			for (int32_t ix = aPosition.m_x - (int32_t)aPack->m_minDistanceToNearBy; ix <= aPosition.m_x + (int32_t)aPack->m_minDistanceToNearBy; ix++)
			{
				int32_t dx = aPosition.m_x - ix;
				int32_t dy = aPosition.m_y - iy;
				int32_t distanceSquared = dx * dx + dy * dy;
				if (distanceSquared <= minDistanceSquared && m_enemyPositions.contains({ ix, iy }))
					return false;
			}
		}

		return true;
	}

	void
	World::Builder::PlacePack(
		const Pack*						aPack,
		const Vec2&						aPosition,
		uint32_t						aLevel,
		uint32_t						aFactionId)
	{
		bool firstEnemy = true;

		for (uint32_t tagContextId : aPack->m_tagContextIds)
		{
			uint32_t entityId = 0;

			{
				EnemyTypeKey enemyTypeKey;
				enemyTypeKey.m_tagContextId = tagContextId;
				enemyTypeKey.m_level = aLevel;
				enemyTypeKey.m_elite = aPack->m_elite;
				auto i = m_enemyTypeTable.find(enemyTypeKey);
				if(i != m_enemyTypeTable.cend())
				{
					entityId = i->second;
				}
				else
				{
					const TaggedData::QueryResult* result = m_mapGeneratorRuntime->m_entities->PerformQueryWithTagContext(m_manifest->GetById<Data::TagContext>(tagContextId));
					const TaggedData::QueryResult::Entry* picked = result->TryPickRandomFiltered(m_random, [&](
						const TaggedData::QueryResult::Entry* aEntry) -> bool
					{
						const Data::Entity* entity = m_manifest->GetById<Data::Entity>(aEntry->m_id);
						const Components::CombatPublic* combatPublic = entity->TryGetComponent<Components::CombatPublic>();
						if (combatPublic->m_level == aLevel && combatPublic->m_factionId == aFactionId && combatPublic->IsElite() == aPack->m_elite)
							return true;
						return false;
					});

					TP_CHECK(picked != NULL, "Unable to pick enemy.");
					entityId = picked->m_id;

					m_enemyTypeTable[enemyTypeKey] = entityId;
				}
			}

			std::optional<Vec2> position;

			if (firstEnemy)
			{
				firstEnemy = false;
				position = aPosition;
			}
			else
			{
				static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
				size_t randomOffset = (size_t)m_random() % 4;
				for (size_t i = 0; i < 4; i++)
				{
					size_t j = (i + randomOffset) % 4;
					Vec2 p = { aPosition.m_x + NEIGHBORS[j].m_x, aPosition.m_y + NEIGHBORS[j].m_y };
					if (!m_entitySpawnPositions.contains(p) && m_walkable.contains(p))
					{
						position = p;
						break;
					}
				}
			}

			if (position.has_value())
			{
				MapData::EntitySpawn entitySpawn;
				entitySpawn.m_entityId = entityId;
				entitySpawn.m_mapEntitySpawnId = aPack->m_mapEntitySpawnId;
				entitySpawn.m_x = position->m_x;
				entitySpawn.m_y = position->m_y;
				m_entitySpawns.push_back(entitySpawn);

				m_entitySpawnPositions.insert(position.value());

				m_enemyPositions.insert(position.value());
			}
		}

		if(aPack->m_randomObject.has_value() && Roll(1, 1000) <= aPack->m_randomObject->m_probability)
		{
			static const Vec2 NEIGHBORS[8] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 }, { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
			size_t randomOffset = (size_t)m_random() % 8;
			std::optional<Vec2> position;

			for (size_t i = 0; i < 8; i++)
			{
				size_t j = (i + randomOffset) % 8;

				Vec2 p = { aPosition.m_x + NEIGHBORS[j].m_x, aPosition.m_y + NEIGHBORS[j].m_y };
				if (!m_entitySpawnPositions.contains(p) && m_walkable.contains(p))
				{
					position = p;
					break;
				}
			}

			if(position.has_value())
			{
				MapData::EntitySpawn entitySpawn;
				entitySpawn.m_mapEntitySpawnId = aPack->m_randomObject->m_mapEntitySpawnId;
				entitySpawn.m_x = position->m_x;
				entitySpawn.m_y = position->m_y;
				m_entitySpawns.push_back(entitySpawn);

				m_entitySpawnPositions.insert(position.value());
			}
		}
	}

	void			
	World::Builder::InitPlayerSpawns()
	{	
		if(!m_bossDistanceCombined)
		{
			// No bosses, just put player spawn some random place. Only allow one player spawn
			if(m_playerSpawns.size() > 1)
				m_playerSpawns.resize(1);

			if (m_playerSpawns.size() == 1)
				m_playerSpawns[0]->m_position = GetFirstWalkableOutsideCaves();

			if(m_playerSpawns.size() > 0)
			{
				m_playerSpawnDistanceCombined = std::make_unique<DistanceField>((int32_t)m_width, (int32_t)m_height);
				m_playerSpawnDistanceCombined->Generate({ m_playerSpawns[0]->m_position }, m_walkable, UINT32_MAX);
			}
		}
		else
		{
			// First we'll identify positions that are furthest away from any boss
			std::vector<Vec2> furthestPositions;
			m_bossDistanceCombined->GetPositionsMoreThanValue(m_bossDistanceCombined->GetMax() - 16, furthestPositions);
			RemoveCavePositions(furthestPositions);

			std::vector<Vec2> spawnPositions;
			int32_t minSpawnDistanceSquared = 32 * 32;

			for(std::unique_ptr<PlayerSpawn>& playerSpawn : m_playerSpawns)
			{
				// Try finding a spot from positions furthest from bosses
				std::optional<Vec2> position;
				while(furthestPositions.size() > 0 && !position.has_value())
				{
					uint32_t i = Roll(0, (uint32_t)furthestPositions.size() - 1);
					Vec2 tryPosition = furthestPositions[i];
					Helpers::RemoveCyclicFromVector(furthestPositions, (size_t)i);

					bool canUse = true;
					for(const Vec2& spawnPosition : spawnPositions)
					{
						int32_t dx = spawnPosition.m_x - tryPosition.m_x;
						int32_t dy = spawnPosition.m_y - tryPosition.m_y;
						int32_t distanceSquared = dx * dx + dy * dy;
						if(distanceSquared < minSpawnDistanceSquared)
						{
							canUse = false;
							break;
						}
					}

					if(canUse)
						position = tryPosition;
				}

				if(!position.has_value() && m_playerSpawnDistanceCombined)
				{
					assert(furthestPositions.size() == 0);

					std::vector<Vec2> positions;
					m_playerSpawnDistanceCombined->GetPositionsMoreThanValue(m_playerSpawnDistanceCombined->GetMax() - 8, positions);
					RemoveCavePositions(positions);
					if(positions.size() > 0)
					{
						uint32_t i = Roll(0, (uint32_t)positions.size() - 1);
						position = positions[i];
					}
				}

				if(position.has_value())
				{
					spawnPositions.push_back(position.value());

					playerSpawn->m_position = position.value();

					DistanceField distanceField((int32_t)m_width, (int32_t)m_height);
					distanceField.Generate({ playerSpawn->m_position }, m_walkable, UINT32_MAX);

					for(int32_t y = 0; y < distanceField.m_height; y++)
					{
						for (int32_t x = 0; x < distanceField.m_width; x++)
						{
							int32_t i = x + y * distanceField.m_width;
							if(m_levelMap[i].m_boss != NULL)
								distanceField.m_data[i] = UINT32_MAX;
						}
					}

					if(!m_playerSpawnDistanceCombined)
						m_playerSpawnDistanceCombined = std::make_unique<DistanceField>((int32_t)m_width, (int32_t)m_height);

					m_playerSpawnDistanceCombined->CombineMin(&distanceField);
				}
			}		
		}
	}

	void
	World::Builder::InitEnemies()
	{
		// Needs bosses for levels and faction, and player spawns to keep away from
		if(m_addEnemies.empty() || m_bosses.empty() || m_playerSpawns.empty())
			return;

		std::vector<Vec2> playerSpawnPositions;
		for(const std::unique_ptr<PlayerSpawn>& playerSpawn : m_playerSpawns)
			playerSpawnPositions.push_back(playerSpawn->m_position);

		DistanceField playerSpawnDistance((int32_t)m_width, (int32_t)m_height);
		playerSpawnDistance.Generate(playerSpawnPositions, m_walkable, UINT32_MAX);

		for(const Enemies* enemies : m_addEnemies)
		{
			for (int32_t y = 0; y < (int32_t)m_height; y++)
			{
				for (int32_t x = 0; x < (int32_t)m_width; x++)
				{
					size_t index = (size_t)(x + y * (int32_t)m_width);
					uint32_t distance = playerSpawnDistance.m_data[index];
					if(distance == UINT32_MAX || distance <= enemies->m_minPlayerSpawnDistance || m_entitySpawnPositions.contains({ x, y }))
						continue;

					const Pack* pickedPack = NULL;

					for (const Pack& pack : enemies->m_packs)
					{
						if (Roll(1, 1000) <= pack.m_probability && CanPlacePack(&pack, { x, y }))
						{
							pickedPack = &pack;
							break;
						}
					}

					if (pickedPack != NULL)
					{
						// Level and faction from the boss with the most influence here. Outside the influence of all bosses, the lowest
						// level and the faction of the first boss.
						const LevelMapPoint& point = m_levelMap[index];
						uint32_t factionId = point.m_boss != NULL ? point.m_boss->m_factionId : m_bosses[0]->m_factionId;
						PlacePack(pickedPack, { x, y }, point.m_level, factionId);
					}
				}
			}
		}
	}

	void			
	World::Builder::InitSpecialEntities()
	{
		assert(m_playerSpawns.size() > 0);

		for(const SpecialEntity& specialEntity : m_specialEntities)
		{
			TP_CHECK(specialEntity.m_mapEntitySpawns.size() > 0, "No map entity spawns defined.");
			uint32_t mapEntitySpawnId = specialEntity.m_mapEntitySpawns[Roll(0, (uint32_t)specialEntity.m_mapEntitySpawns.size() - 1)];
			Vec2 position = *m_walkable.cbegin();

			std::vector<Vec2> positions;
			GetPlacementPositions(specialEntity.m_placement, specialEntity.m_range, positions);
			if (positions.size() > 0)
				position = positions[Roll(0, (uint32_t)positions.size() - 1)];

			MapData::EntitySpawn t;
			t.m_mapEntitySpawnId = mapEntitySpawnId;
			t.m_x = position.m_x;
			t.m_y = position.m_y;
			m_entitySpawns.push_back(t);
			m_entitySpawnPositions.insert({ t.m_x, t.m_y });
			m_specialEntityPositions.insert(position);
		}
	}

	const Data::Terrain::Tile*
	World::Builder::GetTerrainTile(
		uint32_t							aTerrainId,
		size_t								aIndex) const
	{
		const Data::Terrain* terrain = m_manifest->GetById<Data::Terrain>(aTerrainId);

		for(const std::unique_ptr<Data::Terrain::Tile>& tile : terrain->m_tiles)
		{
			bool ok = true;

			for(const Data::Terrain::TileThreshold& tileThreshold : tile->m_thresholds)
			{
				TP_CHECK(TerrainModifier::ValidateId(tileThreshold.m_terrainModifierId), "Invalid terrain modifier.");
				const std::vector<int32_t>& terrainModifierMap = m_terrainModifierMaps[tileThreshold.m_terrainModifierId];
				if(!terrainModifierMap.empty() && !tileThreshold.Check(terrainModifierMap[aIndex]))
				{
					ok = false;
					break;
				}
			}

			if(ok)
				return tile.get();
		}

		return NULL;
	}

	uint32_t
	World::Builder::GetTerrainTileSpriteId(
		uint32_t							aTerrainId,
		size_t								aIndex) const
	{
		const Data::Terrain::Tile* tile = GetTerrainTile(aTerrainId, aIndex);
		uint32_t tileSpriteId = tile != NULL ? tile->m_tileSpriteId : 0;

		if(tileSpriteId != 0)
		{
			const LevelMapPoint& levelMapPoint = m_levelMap[aIndex];
			if(levelMapPoint.m_boss != NULL && levelMapPoint.m_influence > levelMapPoint.m_boss->m_influenceTileTransformThreshold && levelMapPoint.m_boss->m_factionId)
			{
				const Data::Faction* faction = m_manifest->GetById<Data::Faction>(levelMapPoint.m_boss->m_factionId);
				uint32_t toTileSpriteId = faction->GetInfluenceTileTransform(tileSpriteId);
				if(toTileSpriteId != 0)
					tileSpriteId = toTileSpriteId;
			}
		}

		return tileSpriteId;
	}

	uint32_t
	World::Builder::GetCaveEntranceSpriteId(
		size_t								aIndex) const
	{
		const Data::Terrain::Tile* tile = GetTerrainTile(m_terrainMap[aIndex], aIndex);
		return tile != NULL ? tile->m_caveEntranceSpriteId : 0;
	}

	void
	World::Builder::InitCaves()
	{
		if(m_addCaves.empty())
			return;

		static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
		static const uint32_t MAX_ATTEMPTS = 64;

		int32_t width = (int32_t)m_width;
		int32_t height = (int32_t)m_height;
		size_t mapSize = (size_t)(width * height);

		m_caveMap.resize(mapSize, CAVE_MAP_NONE);
		m_caveCoverTerrainMap.resize(mapSize, 0);
		m_caveStampMap.resize(mapSize, 0);
		m_caveParentMap.resize(mapSize, -1);
		m_caveCostMap.resize(mapSize, 0);
		m_caveStepsMap.resize(mapSize, 0);

		std::vector<uint8_t> walkableMap(mapSize, 0);
		for(const Vec2& position : m_walkable)
			walkableMap[(size_t)(position.m_x + position.m_y * width)] = 1;

		std::vector<MapData::EntitySpawn> caveEntitySpawns;

		for(const Caves& caves : m_addCaves)
		{
			auto isHost = [&](
				size_t		aIndex) -> bool
			{
				if(m_caveMap[aIndex] != CAVE_MAP_NONE)
					return false;
				for(uint32_t terrainId : caves.m_terrainIds)
				{
					if(m_terrainMap[aIndex] == terrainId)
						return true;
				}
				return false;
			};

			// How deep each position is inside the terrain to dig into: 1 means next to something else (or the map edge)
			std::vector<uint32_t> depth(mapSize, 0);
			{
				std::vector<Vec2> queue;

				for(int32_t y = 0; y < height; y++)
				{
					for(int32_t x = 0; x < width; x++)
					{
						size_t i = (size_t)(x + y * width);
						if(!isHost(i))
							continue;

						bool edge = false;
						for(const Vec2& offset : NEIGHBORS)
						{
							Vec2 n = { x + offset.m_x, y + offset.m_y };
							if(n.m_x < 0 || n.m_y < 0 || n.m_x >= width || n.m_y >= height || !isHost((size_t)(n.m_x + n.m_y * width)))
							{
								edge = true;
								break;
							}
						}

						if(edge)
						{
							depth[i] = 1;
							queue.push_back({ x, y });
						}
					}
				}

				for(size_t j = 0; j < queue.size(); j++)
				{
					Vec2 p = queue[j];
					uint32_t d = depth[(size_t)(p.m_x + p.m_y * width)];

					for(const Vec2& offset : NEIGHBORS)
					{
						Vec2 n = { p.m_x + offset.m_x, p.m_y + offset.m_y };
						if(n.m_x < 0 || n.m_y < 0 || n.m_x >= width || n.m_y >= height)
							continue;
						size_t k = (size_t)(n.m_x + n.m_y * width);
						if(depth[k] == 0 && isHost(k))
						{
							depth[k] = d + 1;
							queue.push_back(n);
						}
					}
				}
			}

			// Entrance candidates: south facing edge with the main walkable area right below, dug-into terrain on both sides
			// (which is left alone), and a tile that has a cave entrance
			std::vector<Vec2> candidates;

			for(int32_t y = 0; y < height - 1; y++)
			{
				for(int32_t x = 1; x < width - 1; x++)
				{
					size_t i = (size_t)(x + y * width);
					if(isHost(i) && depth[i] == 1 && walkableMap[i + (size_t)width] != 0 && isHost(i - 1) && isHost(i + 1) && GetCaveEntranceSpriteId(i) != 0)
						candidates.push_back({ x, y });
				}
			}

			uint32_t count = Roll(caves.m_count.m_min, caves.m_count.m_max);

			for(uint32_t i = 0; i < count && candidates.size() > 0; i++)
			{
				for(uint32_t attempt = 0; attempt < MAX_ATTEMPTS && candidates.size() > 0; attempt++)
				{
					uint32_t candidateIndex = Roll(0, (uint32_t)candidates.size() - 1);
					Vec2 entrance = candidates[candidateIndex];
					Helpers::RemoveCyclicFromVector(candidates, (size_t)candidateIndex);

					if(TryDigCave(caves, entrance, depth, walkableMap, caveEntitySpawns))
						break;
				}
			}
		}

		// Remove entity spawns (gathering nodes etc) that were in what got dug out
		std::erase_if(m_entitySpawns, [&](
			const MapData::EntitySpawn& aEntitySpawn)
		{
			uint8_t value = m_caveMap[(size_t)(aEntitySpawn.m_x + aEntitySpawn.m_y * width)];
			return value == CAVE_MAP_FLOOR || value == CAVE_MAP_ENTRANCE;
		});

		std::erase_if(m_entitySpawnPositions, [&](
			const Vec2& aPosition)
		{
			return IsInCave(aPosition);
		});

		// Then add the ones made for the caves
		for(const MapData::EntitySpawn& entitySpawn : caveEntitySpawns)
		{
			m_entitySpawns.push_back(entitySpawn);
			m_entitySpawnPositions.insert({ entitySpawn.m_x, entitySpawn.m_y });
		}
	}

	bool
	World::Builder::TryDigCave(
		const Caves&						aCaves,
		const Vec2&							aEntrance,
		const std::vector<uint32_t>&		aDepth,
		std::vector<uint8_t>&				aWalkableMap,
		std::vector<MapData::EntitySpawn>&	aOutEntitySpawns)
	{
		static const Vec2 DIRECTIONS[4] = { { 0, -1 }, { 1, 0 }, { -1, 0 }, { 0, 1 } }; // North, east, west, south
		static const Vec2 NEIGHBORS_8[8] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } };

		int32_t width = (int32_t)m_width;
		int32_t height = (int32_t)m_height;
		uint32_t innerDepth = aCaves.m_margin + 1;

		auto isHost = [&](
			size_t		aIndex) -> bool
		{
			if(m_caveMap[aIndex] != CAVE_MAP_NONE)
				return false;
			for(uint32_t terrainId : aCaves.m_terrainIds)
			{
				if(m_terrainMap[aIndex] == terrainId)
					return true;
			}
			return false;
		};

		auto isInMap = [&](
			const Vec2&	aP) -> bool
		{
			return aP.m_x >= 0 && aP.m_y >= 0 && aP.m_x < width && aP.m_y < height;
		};

		auto toIndex = [&](
			const Vec2&	aP) -> size_t
		{
			return (size_t)(aP.m_x + aP.m_y * width);
		};

		// Where the walkers may dig: deep enough inside, so the margin is kept
		auto isInner = [&](
			const Vec2&	aP) -> bool
		{
			return isInMap(aP) && isHost(toIndex(aP)) && aDepth[toIndex(aP)] >= innerDepth;
		};

		// Entrance must still be valid (an earlier cave might have taken it or its sides)
		{
			size_t i = toIndex(aEntrance);
			if(!isHost(i) || aWalkableMap[i + (size_t)width] == 0 || !isHost(i - 1) || !isHost(i + 1))
				return false;
		}

		// Neck: straight north from the entrance until reaching the inner area. Every position in it must have dug-into
		// terrain on all sides (depth 2 or more), so it doesn't open up anywhere else.
		std::vector<int32_t> neck;
		std::optional<Vec2> start;

		for(int32_t y = aEntrance.m_y - 1; y >= 0 && !start.has_value() && aEntrance.m_y - y <= (int32_t)innerDepth + 1; y--)
		{
			Vec2 p = { aEntrance.m_x, y };
			size_t i = toIndex(p);
			if(!isHost(i) || aDepth[i] < 2)
				return false;

			neck.push_back((int32_t)i);

			if(aDepth[i] >= innerDepth)
				start = p;
		}

		if(!start.has_value())
			return false;

		// Is there room enough?
		{
			m_caveStamp++;
			std::vector<Vec2> queue = { start.value() };
			m_caveStampMap[toIndex(start.value())] = m_caveStamp;

			for(size_t j = 0; j < queue.size() && queue.size() < (size_t)aCaves.m_size.m_min; j++)
			{
				for(const Vec2& offset : DIRECTIONS)
				{
					Vec2 n = { queue[j].m_x + offset.m_x, queue[j].m_y + offset.m_y };
					if(isInner(n) && m_caveStampMap[toIndex(n)] != m_caveStamp)
					{
						m_caveStampMap[toIndex(n)] = m_caveStamp;
						queue.push_back(n);
					}
				}
			}

			if(queue.size() < (size_t)aCaves.m_size.m_min)
				return false;
		}

		// Dig with walkers: mostly straight, sometimes turning, rarely back towards the entrance. Now and then a small
		// chamber, sometimes a new walker branches off.
		m_caveStamp++;
		uint32_t floorStamp = m_caveStamp;
		std::vector<int32_t> floor;

		auto isFloor = [&](
			const Vec2&	aP) -> bool
		{
			return isInMap(aP) && m_caveStampMap[toIndex(aP)] == floorStamp;
		};

		auto dig = [&](
			const Vec2&	aP)
		{
			size_t i = toIndex(aP);
			if(m_caveStampMap[i] != floorStamp)
			{
				m_caveStampMap[i] = floorStamp;
				floor.push_back((int32_t)i);
			}
		};

		auto pickDirection = [&]() -> uint32_t
		{
			uint32_t roll = Roll(0, 7);
			if(roll < 3)
				return 0; // North
			if(roll < 5)
				return 1; // East
			if(roll < 7)
				return 2; // West
			return 3; // South
		};

		for(int32_t i : neck)
			dig({ i % width, i / width });

		struct Walker
		{
			Vec2		m_position;
			uint32_t	m_direction = 0;
		};

		std::vector<Walker> walkers = { { start.value(), 0 } };
		uint32_t targetSize = Roll(aCaves.m_size.m_min, aCaves.m_size.m_max) + (uint32_t)neck.size();
		uint32_t steps = 0;
		uint32_t maxSteps = targetSize * 30;

		while(floor.size() < (size_t)targetSize && steps < maxSteps)
		{
			for(size_t w = 0; w < walkers.size() && floor.size() < (size_t)targetSize; w++)
			{
				steps++;

				if(Roll(0, 99) < 35)
					walkers[w].m_direction = pickDirection();

				Vec2 next = { walkers[w].m_position.m_x + DIRECTIONS[walkers[w].m_direction].m_x, walkers[w].m_position.m_y + DIRECTIONS[walkers[w].m_direction].m_y };
				if(!isInner(next) && !isFloor(next))
				{
					walkers[w].m_direction = pickDirection();
					continue;
				}

				walkers[w].m_position = next;
				dig(next);

				if(Roll(0, 99) < 6)
				{
					int32_t radius = (int32_t)Roll(1, 3);
					for(int32_t y = -radius; y <= radius; y++)
					{
						for(int32_t x = -radius; x <= radius; x++)
						{
							Vec2 p = { next.m_x + x, next.m_y + y };
							if(x * x + y * y <= radius * radius + 1 && isInner(p))
								dig(p);
						}
					}
				}

				if(walkers.size() < 4 && Roll(0, 99) < 2)
					walkers.push_back({ next, pickDirection() });
			}
		}

		// Smooth: fill in walls mostly surrounded by floor and remove floor sticking out into walls (inner area only, so
		// the neck and the margin are left alone)
		for(uint32_t pass = 0; pass < 2; pass++)
		{
			Vec2 min = { INT32_MAX, INT32_MAX };
			Vec2 max = { INT32_MIN, INT32_MIN };
			for(int32_t i : floor)
			{
				min = { Base::Min(min.m_x, i % width), Base::Min(min.m_y, i / width) };
				max = { Base::Max(max.m_x, i % width), Base::Max(max.m_y, i / width) };
			}

			std::vector<Vec2> addFloor;
			std::vector<Vec2> removeFloor;

			for(int32_t y = min.m_y - 1; y <= max.m_y + 1; y++)
			{
				for(int32_t x = min.m_x - 1; x <= max.m_x + 1; x++)
				{
					Vec2 p = { x, y };
					if(!isInner(p) || p == start.value())
						continue;

					uint32_t floorNeighbors = 0;
					for(const Vec2& offset : NEIGHBORS_8)
					{
						if(isFloor({ x + offset.m_x, y + offset.m_y }))
							floorNeighbors++;
					}

					if(isFloor(p) && floorNeighbors <= 2)
						removeFloor.push_back(p);
					else if(!isFloor(p) && floorNeighbors >= 5)
						addFloor.push_back(p);
				}
			}

			for(const Vec2& p : removeFloor)
				m_caveStampMap[toIndex(p)] = 0;
			for(const Vec2& p : addFloor)
				m_caveStampMap[toIndex(p)] = floorStamp;

			// Keep only what's connected to the entrance
			m_caveStamp++;
			uint32_t connectedStamp = m_caveStamp;
			std::vector<Vec2> queue;
			for(int32_t i : neck)
			{
				queue.push_back({ i % width, i / width });
				m_caveStampMap[(size_t)i] = connectedStamp;
			}

			for(size_t j = 0; j < queue.size(); j++)
			{
				for(const Vec2& offset : DIRECTIONS)
				{
					Vec2 n = { queue[j].m_x + offset.m_x, queue[j].m_y + offset.m_y };
					if(isInMap(n) && m_caveStampMap[toIndex(n)] == floorStamp)
					{
						m_caveStampMap[toIndex(n)] = connectedStamp;
						queue.push_back(n);
					}
				}
			}

			floorStamp = connectedStamp;
			floor.clear();
			for(const Vec2& p : queue)
				floor.push_back((int32_t)toIndex(p));
		}

		// Smoothing and the connectivity check can cut away most of it, so check the size again
		if(floor.size() < neck.size() + (size_t)aCaves.m_size.m_min)
			return false;

		// More entrances
		std::vector<Vec2> entrances = { aEntrance };
		AddCaveEntrances(aCaves, floor, entrances, aDepth, aWalkableMap);

		// Apply
		for(int32_t i : floor)
		{
			m_caveCoverTerrainMap[(size_t)i] = m_terrainMap[(size_t)i];
			m_terrainMap[(size_t)i] = aCaves.m_floorTerrainId;
			m_caveMap[(size_t)i] = CAVE_MAP_FLOOR;
			aWalkableMap[(size_t)i] = 1;
			m_walkable.insert({ i % width, i / width });
		}

		// Entrances are dug out too, but get a cave entrance made for the tile that was there as cover
		for(const Vec2& entrance : entrances)
		{
			size_t i = toIndex(entrance);
			uint32_t caveEntranceSpriteId = GetCaveEntranceSpriteId(i);
			assert(caveEntranceSpriteId != 0);
			m_terrainMap[i] = aCaves.m_floorTerrainId;
			m_caveMap[i] = CAVE_MAP_ENTRANCE;
			m_caveEntranceCovers.push_back({ i, caveEntranceSpriteId });
			aWalkableMap[i] = 1;
			m_walkable.insert(entrance);

			size_t approach = i + (size_t)width;
			if(m_caveMap[approach] == CAVE_MAP_NONE)
				m_caveMap[approach] = CAVE_MAP_APPROACH;
		}

		if(aCaves.m_entitySpawns.size() > 0)
			AddCaveEntitySpawns(aCaves, floor, entrances, aOutEntitySpawns);

		return true;
	}

	void
	World::Builder::AddCaveEntrances(
		const Caves&						aCaves,
		std::vector<int32_t>&				aFloor,
		std::vector<Vec2>&					aEntrances,
		const std::vector<uint32_t>&		aDepth,
		const std::vector<uint8_t>&			aWalkableMap)
	{
		uint32_t entranceCount = Roll(aCaves.m_entranceCount.m_min, aCaves.m_entranceCount.m_max);
		if(entranceCount <= 1)
			return;

		static const Vec2 DIRECTIONS[4] = { { 0, -1 }, { 1, 0 }, { -1, 0 }, { 0, 1 } };

		int32_t width = (int32_t)m_width;
		int32_t height = (int32_t)m_height;

		auto isHost = [&](
			size_t		aIndex) -> bool
		{
			if(m_caveMap[aIndex] != CAVE_MAP_NONE)
				return false;
			for(uint32_t terrainId : aCaves.m_terrainIds)
			{
				if(m_terrainMap[aIndex] == terrainId)
					return true;
			}
			return false;
		};

		auto toIndex = [&](
			const Vec2&	aP) -> size_t
		{
			return (size_t)(aP.m_x + aP.m_y * width);
		};

		// Tunnels go through the dug-into terrain without touching anything else (depth 2 or more)
		m_caveStamp++;
		uint32_t floorStamp = m_caveStamp;
		for(int32_t i : aFloor)
			m_caveStampMap[(size_t)i] = floorStamp;

		m_caveStamp++;
		uint32_t visitedStamp = m_caveStamp;

		// Cheapest paths out from the cave, with a random cost per position so tunnels wind a bit. The cost comes from a
		// hash of the position, so only one random number is used.
		uint32_t tunnelSeed = Roll(0, 0x7FFFFFFF);

		auto getTunnelCost = [&](
			size_t		aIndex) -> uint32_t
		{
			uint32_t h = ((uint32_t)aIndex * 2654435761u) ^ tunnelSeed;
			h ^= h >> 15;
			h *= 2246822519u;
			h ^= h >> 13;
			return 1 + h % 6;
		};

		std::set<std::pair<uint32_t, int32_t>> open; // Cost and position index, ordered for the same result on all platforms
		std::vector<int32_t> visited;

		for(int32_t i : aFloor)
		{
			m_caveCostMap[(size_t)i] = 0;
			m_caveStepsMap[(size_t)i] = 0;
			m_caveParentMap[(size_t)i] = -1;
			open.insert({ 0, i });
		}

		while(open.size() > 0)
		{
			auto [cost, index] = *open.begin();
			open.erase(open.begin());

			if(cost > m_caveCostMap[(size_t)index])
				continue;

			uint32_t steps = m_caveStepsMap[(size_t)index];
			if(steps >= aCaves.m_maxEntranceTunnel)
				continue;

			Vec2 position = { index % width, index / width };

			for(const Vec2& offset : DIRECTIONS)
			{
				Vec2 n = { position.m_x + offset.m_x, position.m_y + offset.m_y };
				if(n.m_x < 1 || n.m_y < 1 || n.m_x + 1 >= width || n.m_y >= height - 2)
					continue;

				size_t i = toIndex(n);
				if(m_caveStampMap[i] == floorStamp || !isHost(i) || aDepth[i] < 2)
					continue;

				uint32_t neighborCost = cost + getTunnelCost(i);
				if(m_caveStampMap[i] == visitedStamp && neighborCost >= m_caveCostMap[i])
					continue;

				if(m_caveStampMap[i] != visitedStamp)
				{
					m_caveStampMap[i] = visitedStamp;
					visited.push_back((int32_t)i);
				}

				m_caveCostMap[i] = neighborCost;
				m_caveStepsMap[i] = steps + 1;
				m_caveParentMap[i] = index;
				open.insert({ neighborCost, (int32_t)i });
			}
		}

		// Tunnel end positions: right above a valid entrance (same rules as the first one: south facing edge, walkable
		// below, dug-into terrain on both sides)
		std::vector<Vec2> tunnelEnds;

		for(int32_t i : visited)
		{
			size_t e = (size_t)i + (size_t)width;
			if(isHost(e) && aDepth[e] == 1 && aWalkableMap[e + (size_t)width] != 0 && isHost(e - 1) && isHost(e + 1) && GetCaveEntranceSpriteId(e) != 0)
				tunnelEnds.push_back({ i % width, i / width });
		}

		// Pick entrances at random, far enough from each other
		int32_t minDistanceSquared = (int32_t)(aCaves.m_minEntranceDistance * aCaves.m_minEntranceDistance);

		while(aEntrances.size() < (size_t)entranceCount && tunnelEnds.size() > 0)
		{
			uint32_t pick = Roll(0, (uint32_t)tunnelEnds.size() - 1);
			Vec2 tunnelEnd = tunnelEnds[pick];
			Helpers::RemoveCyclicFromVector(tunnelEnds, (size_t)pick);

			Vec2 entrance = { tunnelEnd.m_x, tunnelEnd.m_y + 1 };

			bool farEnough = true;
			for(const Vec2& other : aEntrances)
			{
				int32_t dx = other.m_x - entrance.m_x;
				int32_t dy = other.m_y - entrance.m_y;
				if(dx * dx + dy * dy < minDistanceSquared)
				{
					farEnough = false;
					break;
				}
			}

			if(!farEnough)
				continue;

			// Tunnel back to the cave
			for(int32_t i = (int32_t)toIndex(tunnelEnd); i != -1 && m_caveStampMap[(size_t)i] != floorStamp; i = m_caveParentMap[(size_t)i])
			{
				m_caveStampMap[(size_t)i] = floorStamp;
				aFloor.push_back(i);
			}

			aEntrances.push_back(entrance);
		}
	}

	void
	World::Builder::AddCaveEntitySpawns(
		const Caves&						aCaves,
		const std::vector<int32_t>&			aFloor,
		const std::vector<Vec2>&			aEntrances,
		std::vector<MapData::EntitySpawn>&	aOutEntitySpawns)
	{
		static const Vec2 DIRECTIONS[4] = { { 0, -1 }, { 1, 0 }, { -1, 0 }, { 0, 1 } };

		int32_t width = (int32_t)m_width;

		auto toIndex = [&](
			const Vec2&	aP) -> size_t
		{
			return (size_t)(aP.m_x + aP.m_y * width);
		};

		// Distance from the nearest entrance, walking inside the cave
		m_caveStamp++;
		uint32_t caveStamp = m_caveStamp;
		for(int32_t i : aFloor)
			m_caveStampMap[(size_t)i] = caveStamp;

		m_caveStamp++;
		uint32_t visitedStamp = m_caveStamp;

		std::vector<Vec2> queue;
		for(const Vec2& entrance : aEntrances)
		{
			m_caveStampMap[toIndex(entrance)] = visitedStamp;
			m_caveStepsMap[toIndex(entrance)] = 0;
			queue.push_back(entrance);
		}

		uint32_t maxDistance = 0;

		for(size_t j = 0; j < queue.size(); j++)
		{
			uint32_t distance = m_caveStepsMap[toIndex(queue[j])];

			for(const Vec2& offset : DIRECTIONS)
			{
				Vec2 n = { queue[j].m_x + offset.m_x, queue[j].m_y + offset.m_y };
				if(n.m_x < 0 || n.m_y < 0 || n.m_x >= width || n.m_y >= (int32_t)m_height)
					continue;
				size_t i = toIndex(n);
				if(m_caveStampMap[i] == caveStamp)
				{
					m_caveStampMap[i] = visitedStamp;
					m_caveStepsMap[i] = distance + 1;
					maxDistance = Base::Max(maxDistance, distance + 1);
					queue.push_back(n);
				}
			}
		}

		// Place them, not next to each other
		std::vector<Vec2> placed;

		for(const Caves::EntitySpawns& entitySpawns : aCaves.m_entitySpawns)
		{
			// Rounded inwards, but a tiny cave still gets the nearest distance (and 0 is only the entrances)
			uint32_t minDistance = Base::Max(1u, (entitySpawns.m_range.m_min * maxDistance + 99) / 100);
			uint32_t maxDistanceInRange = Base::Max(minDistance, (entitySpawns.m_range.m_max * maxDistance) / 100);

			uint32_t count = Roll(entitySpawns.m_count.m_min, entitySpawns.m_count.m_max);

			for(uint32_t k = 0; k < count; k++)
			{
				std::vector<Vec2> candidates;

				for(const Vec2& position : queue)
				{
					uint32_t distance = m_caveStepsMap[toIndex(position)];
					if(distance < minDistance || distance > maxDistanceInRange || m_caveMap[toIndex(position)] != CAVE_MAP_FLOOR)
						continue;

					bool nextToPlaced = false;
					for(const Vec2& other : placed)
					{
						if(std::abs(other.m_x - position.m_x) <= 1 && std::abs(other.m_y - position.m_y) <= 1)
						{
							nextToPlaced = true;
							break;
						}
					}

					if(!nextToPlaced)
						candidates.push_back(position);
				}

				if(candidates.empty())
					break;

				Vec2 position = candidates[Roll(0, (uint32_t)candidates.size() - 1)];
				placed.push_back(position);

				MapData::EntitySpawn t;
				t.m_mapEntitySpawnId = entitySpawns.m_mapEntitySpawnIds[Roll(0, (uint32_t)entitySpawns.m_mapEntitySpawnIds.size() - 1)];
				t.m_x = position.m_x;
				t.m_y = position.m_y;
				aOutEntitySpawns.push_back(t);
			}
		}
	}

	bool
	World::Builder::IsInCave(
		const Vec2&							aPosition) const
	{
		if(m_caveMap.empty())
			return false;
		uint8_t value = m_caveMap[(size_t)(aPosition.m_x + aPosition.m_y * (int32_t)m_width)];
		return value == CAVE_MAP_FLOOR || value == CAVE_MAP_ENTRANCE;
	}

	void
	World::Builder::RemoveCavePositions(
		std::vector<Vec2>&					aPositions) const
	{
		if(m_caveMap.empty())
			return;

		std::erase_if(aPositions, [&](
			const Vec2& aPosition)
		{
			return IsInCave(aPosition);
		});
	}

	Vec2
	World::Builder::GetFirstWalkableOutsideCaves() const
	{
		for(const Vec2& position : m_walkable)
		{
			if(!IsInCave(position))
				return position;
		}
		return *m_walkable.cbegin();
	}

	void
	World::Builder::GetPlacementPositions(
		SpecialEntity::Placement			aPlacement,
		const std::optional<UIntRange>&		aRange,
		std::vector<Vec2>&					aOut) const
	{
		switch(aPlacement)
		{
		case SpecialEntity::PLACEMENT_FAR_AWAY_FROM_PLAYER_SPAWNS:
			TP_CHECK(m_playerSpawnDistanceCombined, "No player spawn distance field.");
			m_playerSpawnDistanceCombined->GetPositionsMoreThanValue(m_playerSpawnDistanceCombined->GetMax() - 8, aOut);
			break;

		case SpecialEntity::PLACEMENT_IN_PLAYER_SPAWN_DISTANCE_RANGE:
			TP_CHECK(m_playerSpawnDistanceCombined, "No player spawn distance field.");
			TP_CHECK(aRange.has_value(), "Missing spawn distance range.");
			m_playerSpawnDistanceCombined->GetPositionsInRange(aRange->m_min, aRange->m_max, aOut);
			break;

		case SpecialEntity::PLACEMENT_IN_PLAYER_SPAWN_DISTANCE_RANGE_PERCENTAGE:
			{
				TP_CHECK(m_playerSpawnDistanceCombined, "No player spawn distance field.");
				TP_CHECK(aRange.has_value(), "Missing spawn distance range.");
				uint32_t maxDistance = m_playerSpawnDistanceCombined->GetMax();
				m_playerSpawnDistanceCombined->GetPositionsInRange((aRange->m_min * maxDistance) / 100, (aRange->m_max * maxDistance) / 100, aOut);
			}
			break;

		default:
			break;
		}

		RemoveCavePositions(aOut);
	}

	void
	World::Builder::InitStructures()
	{
		if(m_addStructures.empty())
			return;

		m_structureMap.resize((size_t)(m_width * m_height), STRUCTURE_MAP_NONE);

		m_passableMap.resize((size_t)(m_width * m_height), 0);
		for(const Vec2& position : m_walkable)
			m_passableMap[(size_t)(position.m_x + position.m_y * (int32_t)m_width)] = 1;

		// Things structures must never be placed on top of
		std::set<Vec2> protectedPositions = m_specialEntityPositions;

		for(const std::unique_ptr<PlayerSpawn>& playerSpawn : m_playerSpawns)
			protectedPositions.insert(playerSpawn->m_position);

		for(const std::unique_ptr<Boss>& boss : m_bosses)
		{
			protectedPositions.insert(boss->m_position);

			for(const MinorBoss& minorBoss : boss->m_subBosses)
				protectedPositions.insert(minorBoss.m_position);
		}

		// Candidate positions are picked for a random entrance, then the rest of the structure is checked. Number of
		// attempts is limited to keep it cheap, as maps are generated on the fly. If it fails the structure is skipped.
		static const uint32_t MAX_ATTEMPTS = 64;

		for(const Structures& structures : m_addStructures)
		{
			std::vector<Vec2> candidates;
			if(structures.m_placement == SpecialEntity::INVALID_PLACEMENT)
				candidates.assign(m_walkable.cbegin(), m_walkable.cend());
			else
				GetPlacementPositions(structures.m_placement, structures.m_range, candidates);

			uint32_t count = Roll(structures.m_count.m_min, structures.m_count.m_max);

			for(uint32_t i = 0; i < count && candidates.size() > 0; i++)
			{
				const Data::MapSegment* mapSegment = m_manifest->GetById<Data::MapSegment>(structures.m_mapSegmentIds[Roll(0, (uint32_t)structures.m_mapSegmentIds.size() - 1)]);
				TP_CHECK(mapSegment->m_tileMap, "Map segment '%s' has no paletted map.", mapSegment->m_name.c_str());
				TP_CHECK(mapSegment->m_entrances.size() > 0, "Map segment '%s' has no entrances.", mapSegment->m_name.c_str());

				for(uint32_t attempt = 0; attempt < MAX_ATTEMPTS && candidates.size() > 0; attempt++)
				{
					uint32_t candidateIndex = Roll(0, (uint32_t)candidates.size() - 1);
					Vec2 entrancePosition = candidates[candidateIndex];
					Helpers::RemoveCyclicFromVector(candidates, (size_t)candidateIndex);

					const Data::MapSegment::Entrance& entrance = mapSegment->m_entrances[Roll(0, (uint32_t)mapSegment->m_entrances.size() - 1)];
					Vec2 position = { entrancePosition.m_x - entrance.m_position.m_x, entrancePosition.m_y - entrance.m_position.m_y };

					std::vector<int32_t> path;
					if(CanPlaceStructure(mapSegment, position, structures, protectedPositions) && KeepStructureConnected(mapSegment, position, path))
					{
						PlaceStructure(mapSegment, position, structures, path);
						break;
					}
				}
			}
		}

		// Remove entity spawns (monster packs, gathering nodes, etc) that ended up inside structures, on entrances, on paths or on anchors
		std::erase_if(m_entitySpawns, [&](
			const MapData::EntitySpawn& aEntitySpawn)
		{
			return m_structureMap[(size_t)(aEntitySpawn.m_x + aEntitySpawn.m_y * (int32_t)m_width)] != STRUCTURE_MAP_NONE;
		});
	}

	bool
	World::Builder::CanPlaceStructure(
		const Data::MapSegment*				aMapSegment,
		const Vec2&							aPosition,
		const Structures&					aStructures,
		const std::set<Vec2>&				aProtectedPositions) const
	{
		const Data::MapSegment::TileMap* tileMap = aMapSegment->m_tileMap.get();

		if(aPosition.m_x < 0 || aPosition.m_y < 0 || aPosition.m_x + tileMap->m_size.m_x > (int32_t)m_width || aPosition.m_y + tileMap->m_size.m_y > (int32_t)m_height)
			return false;

		// Entrances must be part of the main walkable area (structures already placed have been removed from it)
		for(const Data::MapSegment::Entrance& entrance : aMapSegment->m_entrances)
		{
			Vec2 p = { aPosition.m_x + entrance.m_position.m_x, aPosition.m_y + entrance.m_position.m_y };
			if(!m_walkable.contains(p) || aProtectedPositions.contains(p) || (!m_caveMap.empty() && m_caveMap[(size_t)(p.m_x + p.m_y * (int32_t)m_width)] != CAVE_MAP_NONE))
				return false;
		}

		// Anchors that will get something spawned on them, outside the structure, must be inside the map and free
		for(const Data::MapSegment::Anchor& anchor : aMapSegment->m_anchors)
		{
			if(tileMap->IsSet(anchor.m_position) || aStructures.GetAnchorMapEntitySpawnIds(anchor).empty())
				continue;

			Vec2 p = { aPosition.m_x + anchor.m_position.m_x, aPosition.m_y + anchor.m_position.m_y };
			if(p.m_x < 0 || p.m_y < 0 || p.m_x >= (int32_t)m_width || p.m_y >= (int32_t)m_height)
				return false;
			if(m_structureMap[(size_t)(p.m_x + p.m_y * (int32_t)m_width)] != STRUCTURE_MAP_NONE || aProtectedPositions.contains(p))
				return false;
			if(!m_caveMap.empty() && m_caveMap[(size_t)(p.m_x + p.m_y * (int32_t)m_width)] != CAVE_MAP_NONE)
				return false;
		}

		if(aStructures.m_minDistance > 0)
		{
			Vec2 center = { aPosition.m_x + tileMap->m_size.m_x / 2, aPosition.m_y + tileMap->m_size.m_y / 2 };
			int32_t minDistanceSquared = (int32_t)(aStructures.m_minDistance * aStructures.m_minDistance);

			for(const Structure& structure : m_structures)
			{
				const Data::MapSegment::TileMap* otherTileMap = structure.m_mapSegment->m_tileMap.get();
				int32_t dx = structure.m_position.m_x + otherTileMap->m_size.m_x / 2 - center.m_x;
				int32_t dy = structure.m_position.m_y + otherTileMap->m_size.m_y / 2 - center.m_y;
				if(dx * dx + dy * dy < minDistanceSquared)
					return false;
			}
		}

		// Footprint can't cover other structures, their entrances, or protected entities
		for(int32_t y = 0; y < tileMap->m_size.m_y; y++)
		{
			for(int32_t x = 0; x < tileMap->m_size.m_x; x++)
			{
				if(!tileMap->IsSet({ x, y }))
					continue;

				Vec2 p = { aPosition.m_x + x, aPosition.m_y + y };
				if(m_structureMap[(size_t)(p.m_x + p.m_y * (int32_t)m_width)] != STRUCTURE_MAP_NONE || aProtectedPositions.contains(p))
					return false;
				if(!m_caveMap.empty() && m_caveMap[(size_t)(p.m_x + p.m_y * (int32_t)m_width)] != CAVE_MAP_NONE)
					return false;
			}
		}

		return true;
	}

	void
	World::Builder::PlaceStructure(
		const Data::MapSegment*				aMapSegment,
		const Vec2&							aPosition,
		const Structures&					aStructures,
		const std::vector<int32_t>&			aPath)
	{
		const Data::MapSegment::TileMap* tileMap = aMapSegment->m_tileMap.get();

		Structure t;
		t.m_mapSegment = aMapSegment;
		t.m_position = aPosition;

		for(const Data::MapSegment::Anchor& anchor : aMapSegment->m_anchors)
		{
			const std::vector<uint32_t>& mapEntitySpawnIds = aStructures.GetAnchorMapEntitySpawnIds(anchor);
			if(mapEntitySpawnIds.empty())
				continue;

			Vec2 p = { aPosition.m_x + anchor.m_position.m_x, aPosition.m_y + anchor.m_position.m_y };
			if(p.m_x < 0 || p.m_y < 0 || p.m_x >= (int32_t)m_width || p.m_y >= (int32_t)m_height)
				continue; // Only possible for anchors on the structure itself, which is inside the map

			MapData::EntitySpawn entitySpawn;
			entitySpawn.m_mapEntitySpawnId = mapEntitySpawnIds[Roll(0, (uint32_t)mapEntitySpawnIds.size() - 1)];
			entitySpawn.m_x = p.m_x;
			entitySpawn.m_y = p.m_y;
			t.m_anchorEntitySpawns.push_back(entitySpawn);

			uint8_t& structureMapValue = m_structureMap[(size_t)(p.m_x + p.m_y * (int32_t)m_width)];
			if(structureMapValue == STRUCTURE_MAP_NONE)
				structureMapValue = STRUCTURE_MAP_ANCHOR;
		}

		m_structures.push_back(t);

		std::vector<uint8_t> footprint;
		GetStructureFootprint(aMapSegment, footprint);

		for(int32_t y = 0; y < tileMap->m_size.m_y; y++)
		{
			for(int32_t x = 0; x < tileMap->m_size.m_x; x++)
			{
				if(!tileMap->IsSet({ x, y }))
					continue;

				Vec2 p = { aPosition.m_x + x, aPosition.m_y + y };
				m_structureMap[(size_t)(p.m_x + p.m_y * (int32_t)m_width)] = STRUCTURE_MAP_FOOTPRINT;

				uint8_t footprintValue = footprint[(size_t)(x + y * tileMap->m_size.m_x)];
				if(footprintValue != FOOTPRINT_NONE)
					m_passableMap[(size_t)(p.m_x + p.m_y * (int32_t)m_width)] = footprintValue == FOOTPRINT_WALKABLE ? 1 : 0;

				// Keep later placement (routes, zone boundary entities) off the structure
				m_walkable.erase(p);

				if(m_playerSpawnDistanceCombined)
					m_playerSpawnDistanceCombined->m_data[p.m_x + p.m_y * m_playerSpawnDistanceCombined->m_width] = UINT32_MAX;
			}
		}

		for(const Data::MapSegment::Entrance& entrance : aMapSegment->m_entrances)
		{
			Vec2 p = { aPosition.m_x + entrance.m_position.m_x, aPosition.m_y + entrance.m_position.m_y };
			if(p.m_x >= 0 && p.m_y >= 0 && p.m_x < (int32_t)m_width && p.m_y < (int32_t)m_height)
				m_structureMap[(size_t)(p.m_x + p.m_y * (int32_t)m_width)] = STRUCTURE_MAP_ENTRANCE;
		}

		// Path that keeps the walkable area connected (already marked passable by KeepStructureConnected())
		for(int32_t i : aPath)
		{
			const Data::Terrain* terrain = m_manifest->GetById<Data::Terrain>(m_terrainMap[(size_t)i]);
			if(!terrain->m_walkable)
				m_terrainMap[(size_t)i] = terrain->m_connectConversion;

			if(m_structureMap[(size_t)i] == STRUCTURE_MAP_NONE)
				m_structureMap[(size_t)i] = STRUCTURE_MAP_PATH;

			m_walkable.insert({ i % (int32_t)m_width, i / (int32_t)m_width });
		}
	}

	void
	World::Builder::GetStructureFootprint(
		const Data::MapSegment*				aMapSegment,
		std::vector<uint8_t>&				aOut) const
	{
		const Data::MapSegment::TileMap* tileMap = aMapSegment->m_tileMap.get();
		aOut.resize(tileMap->m_tiles.size(), FOOTPRINT_NONE);

		for(size_t i = 0; i < tileMap->m_tiles.size(); i++)
		{
			uint32_t tileSpriteId = tileMap->m_tiles[i];
			if(tileSpriteId != 0)
			{
				const Data::Sprite* sprite = m_manifest->GetById<Data::Sprite>(tileSpriteId);
				aOut[i] = (sprite->m_info.m_flags & SpriteInfo::FLAG_TILE_WALKABLE) != 0 ? FOOTPRINT_WALKABLE : FOOTPRINT_BLOCKED;
			}
		}

		for(const Data::MapSegment::TileMap::DoodadObject& doodad : tileMap->m_doodads)
		{
			const Data::Doodad* doodadData = m_manifest->GetById<Data::Doodad>(doodad.m_doodadId);
			for(uint32_t spriteId : doodadData->m_spriteIds)
			{
				const Data::Sprite* sprite = m_manifest->GetById<Data::Sprite>(spriteId);
				if((sprite->m_info.m_flags & SpriteInfo::FLAG_TILE_WALKABLE) == 0)
				{
					aOut[(size_t)(doodad.m_position.m_x + doodad.m_position.m_y * tileMap->m_size.m_x)] = FOOTPRINT_BLOCKED;
					break;
				}
			}
		}
	}

	bool
	World::Builder::KeepStructureConnected(
		const Data::MapSegment*				aMapSegment,
		const Vec2&							aPosition,
		std::vector<int32_t>&				aOutPath)
	{
		// The walkable area is connected before the structure is placed. Any path that crossed a position the structure
		// blocks must have entered and left through a passable neighbor of a blocked position. This means that if all of
		// these neighbors are still connected to each other, everything is. First this is checked in a small window
		// around the structure, which is usually enough. If not, the groups found in the window are grown outwards
		// in turns until they meet. A group that runs out of positions to grow into has been cut off from the rest,
		// and a path is made for it around the structure.

		static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
		static const size_t MAX_SEARCH = 4096;

		const Data::MapSegment::TileMap* tileMap = aMapSegment->m_tileMap.get();
		int32_t width = (int32_t)m_width;
		int32_t height = (int32_t)m_height;

		std::vector<uint8_t> footprint;
		GetStructureFootprint(aMapSegment, footprint);

		auto isPassable = [&](
			const Vec2&		aP) -> bool
		{
			int32_t x = aP.m_x - aPosition.m_x;
			int32_t y = aP.m_y - aPosition.m_y;
			if(x >= 0 && y >= 0 && x < tileMap->m_size.m_x && y < tileMap->m_size.m_y)
			{
				uint8_t footprintValue = footprint[(size_t)(x + y * tileMap->m_size.m_x)];
				if(footprintValue != FOOTPRINT_NONE)
					return footprintValue == FOOTPRINT_WALKABLE;
			}
			return m_passableMap[(size_t)(aP.m_x + aP.m_y * width)] != 0;
		};

		auto isInMap = [&](
			const Vec2&		aP) -> bool
		{
			return aP.m_x >= 0 && aP.m_y >= 0 && aP.m_x < width && aP.m_y < height;
		};

		// Passable neighbors of positions that become blocked by the structure
		std::vector<Vec2> neighbors;
		for(int32_t y = 0; y < tileMap->m_size.m_y; y++)
		{
			for(int32_t x = 0; x < tileMap->m_size.m_x; x++)
			{
				Vec2 p = { aPosition.m_x + x, aPosition.m_y + y };
				if(footprint[(size_t)(x + y * tileMap->m_size.m_x)] != FOOTPRINT_BLOCKED || m_passableMap[(size_t)(p.m_x + p.m_y * width)] == 0)
					continue;

				for(const Vec2& offset : NEIGHBORS)
				{
					Vec2 n = { p.m_x + offset.m_x, p.m_y + offset.m_y };
					if(isInMap(n) && isPassable(n))
						neighbors.push_back(n);
				}
			}
		}

		if(neighbors.empty())
			return true; // Doesn't block anything

		if(m_groupMap.empty())
		{
			m_groupMap.resize((size_t)(width * height), 0);
			m_groupStampMap.resize((size_t)(width * height), 0);
		}

		m_groupStamp++;

		auto getGroup = [&](
			const Vec2&		aP) -> uint32_t
		{
			size_t i = (size_t)(aP.m_x + aP.m_y * width);
			return m_groupStampMap[i] == m_groupStamp ? m_groupMap[i] : UINT32_MAX;
		};

		auto setGroup = [&](
			const Vec2&		aP,
			uint32_t		aGroup)
		{
			size_t i = (size_t)(aP.m_x + aP.m_y * width);
			m_groupStampMap[i] = m_groupStamp;
			m_groupMap[i] = aGroup;
		};

		// Flood fill window around the structure, starting from each neighbor that isn't in a group yet
		Vec2 windowMin = { (std::max)(aPosition.m_x - 1, 0), (std::max)(aPosition.m_y - 1, 0) };
		Vec2 windowMax = { (std::min)(aPosition.m_x + tileMap->m_size.m_x, width - 1), (std::min)(aPosition.m_y + tileMap->m_size.m_y, height - 1) };

		std::vector<std::vector<Vec2>> groupQueues;
		size_t searched = 0;

		for(const Vec2& start : neighbors)
		{
			if(getGroup(start) != UINT32_MAX)
				continue;

			uint32_t group = (uint32_t)groupQueues.size();
			groupQueues.push_back({});
			std::vector<Vec2>& groupPositions = groupQueues.back();

			setGroup(start, group);
			groupPositions.push_back(start);

			for(size_t i = 0; i < groupPositions.size(); i++)
			{
				Vec2 p = groupPositions[i];

				for(const Vec2& offset : NEIGHBORS)
				{
					Vec2 n = { p.m_x + offset.m_x, p.m_y + offset.m_y };
					if(n.m_x >= windowMin.m_x && n.m_y >= windowMin.m_y && n.m_x <= windowMax.m_x && n.m_y <= windowMax.m_y && getGroup(n) == UINT32_MAX && isPassable(n))
					{
						setGroup(n, group);
						groupPositions.push_back(n);
					}
				}
			}

			searched += groupPositions.size();
		}

		if(groupQueues.size() == 1)
			return true; // All connected around (or through) the structure

		// Grow the groups outwards in turns (without window limit). Groups that meet are merged.
		size_t groupCount = groupQueues.size();
		std::vector<uint32_t> groupRoots(groupCount);
		std::vector<size_t> groupSizes(groupCount);
		std::vector<size_t> queueHeads(groupCount, 0);
		for(size_t i = 0; i < groupCount; i++)
		{
			groupRoots[i] = (uint32_t)i;
			groupSizes[i] = groupQueues[i].size();
		}

		auto findRoot = [&](
			uint32_t		aGroup) -> uint32_t
		{
			while(groupRoots[aGroup] != aGroup)
			{
				groupRoots[aGroup] = groupRoots[groupRoots[aGroup]];
				aGroup = groupRoots[aGroup];
			}
			return aGroup;
		};

		std::vector<bool> activeRoots(groupCount);
		std::vector<bool> allRoots(groupCount);

		for(;;)
		{
			std::fill(activeRoots.begin(), activeRoots.end(), false);
			std::fill(allRoots.begin(), allRoots.end(), false);
			size_t activeRootCount = 0;
			size_t rootCount = 0;

			for(size_t i = 0; i < groupCount; i++)
			{
				uint32_t root = findRoot((uint32_t)i);

				if(!allRoots[root])
				{
					allRoots[root] = true;
					rootCount++;
				}

				if(queueHeads[i] < groupQueues[i].size() && !activeRoots[root])
				{
					activeRoots[root] = true;
					activeRootCount++;
				}
			}

			if(rootCount == 1)
				return true; // Groups met, all connected

			if(activeRootCount <= 1)
				break; // Everything else has been cut off

			for(size_t i = 0; i < groupCount; i++)
			{
				if(queueHeads[i] == groupQueues[i].size())
					continue;

				Vec2 p = groupQueues[i][queueHeads[i]++];

				for(const Vec2& offset : NEIGHBORS)
				{
					Vec2 n = { p.m_x + offset.m_x, p.m_y + offset.m_y };
					if(!isInMap(n) || !isPassable(n))
						continue;

					uint32_t otherGroup = getGroup(n);
					if(otherGroup == UINT32_MAX)
					{
						setGroup(n, (uint32_t)i);
						groupQueues[i].push_back(n);
						groupSizes[i]++;
						searched++;
					}
					else
					{
						uint32_t root = findRoot((uint32_t)i);
						uint32_t otherRoot = findRoot(otherGroup);
						if(root != otherRoot)
							groupRoots[(std::max)(root, otherRoot)] = (std::min)(root, otherRoot);
					}
				}
			}

			if(searched > MAX_SEARCH)
				return false; // Too expensive, try somewhere else
		}

		// The main area is the group still growing (or the largest if they all stopped at the same time)
		uint32_t mainRoot = UINT32_MAX;
		{
			std::vector<size_t> rootSizes(groupCount, 0);
			for(size_t i = 0; i < groupCount; i++)
				rootSizes[findRoot((uint32_t)i)] += groupSizes[i];

			for(uint32_t i = 0; i < (uint32_t)groupCount; i++)
			{
				if(activeRoots[i])
				{
					mainRoot = i;
					break;
				}

				if(allRoots[i] && (mainRoot == UINT32_MAX || rootSizes[i] > rootSizes[mainRoot]))
					mainRoot = i;
			}
		}

		// Connect each cut off group with a path
		std::vector<bool> unconnectedRoots(groupCount, false);
		for(uint32_t i = 0; i < (uint32_t)groupCount; i++)
			unconnectedRoots[i] = allRoots[i] && i != mainRoot;

		for(uint32_t i = 0; i < (uint32_t)groupCount; i++)
		{
			if(!unconnectedRoots[i])
				continue;

			if(!FindStructurePocketPath(aMapSegment, aPosition, footprint, groupRoots, unconnectedRoots, i, aOutPath))
			{
				// Undo paths made for other groups
				for(int32_t j : aOutPath)
					m_passableMap[(size_t)j] = 0;
				aOutPath.clear();
				return false;
			}

			unconnectedRoots[i] = false;
		}

		return true;
	}

	bool
	World::Builder::FindStructurePocketPath(
		const Data::MapSegment*				aMapSegment,
		const Vec2&							aPosition,
		const std::vector<uint8_t>&			aFootprint,
		const std::vector<uint32_t>&		aGroupRoots,
		const std::vector<bool>&			aUnconnectedRoots,
		uint32_t							aPocketRoot,
		std::vector<int32_t>&				aOutPath)
	{
		// Cheapest path in a band around the structure from the pocket to anything passable that isn't cut off. Already
		// passable positions cost nothing, other terrain costs its connect cost plus noise and is converted like when 
		// walkable areas are connected.

		static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
		static const int32_t BAND = 3;

		const Data::MapSegment::TileMap* tileMap = aMapSegment->m_tileMap.get();
		int32_t width = (int32_t)m_width;

		Vec2 bandMin = { (std::max)(aPosition.m_x - BAND, 0), (std::max)(aPosition.m_y - BAND, 0) };
		Vec2 bandMax = { (std::min)(aPosition.m_x + tileMap->m_size.m_x - 1 + BAND, width - 1), (std::min)(aPosition.m_y + tileMap->m_size.m_y - 1 + BAND, (int32_t)m_height - 1) };
		Vec2 bandSize = { bandMax.m_x - bandMin.m_x + 1, bandMax.m_y - bandMin.m_y + 1 };

		auto getRoot = [&](
			size_t			aMapIndex) -> uint32_t
		{
			if(m_groupStampMap[aMapIndex] != m_groupStamp)
				return UINT32_MAX;
			uint32_t group = m_groupMap[aMapIndex];
			while(aGroupRoots[group] != group)
				group = aGroupRoots[group];
			return group;
		};

		// Cost of entering a position, UINT32_MAX if it can't be part of the path
		auto getCost = [&](
			const Vec2&		aP,
			bool&			aOutPassable) -> uint32_t
		{
			aOutPassable = false;

			int32_t x = aP.m_x - aPosition.m_x;
			int32_t y = aP.m_y - aPosition.m_y;
			if(x >= 0 && y >= 0 && x < tileMap->m_size.m_x && y < tileMap->m_size.m_y)
			{
				uint8_t footprintValue = aFootprint[(size_t)(x + y * tileMap->m_size.m_x)];
				if(footprintValue == FOOTPRINT_BLOCKED)
					return UINT32_MAX;
				if(footprintValue == FOOTPRINT_WALKABLE)
				{
					aOutPassable = true;
					return 0;
				}
			}

			size_t i = (size_t)(aP.m_x + aP.m_y * width);
			if(!m_caveMap.empty() && (m_caveMap[i] == CAVE_MAP_FLOOR || m_caveMap[i] == CAVE_MAP_ENTRANCE))
				return UINT32_MAX;

			if(m_passableMap[i] != 0)
			{
				aOutPassable = true;
				return 0;
			}

			if(m_structureMap[i] == STRUCTURE_MAP_FOOTPRINT)
				return UINT32_MAX; // Another structure

			// Carving right next to a cave would open it up from the side
			if(!m_caveMap.empty())
			{
				for(const Vec2& offset : NEIGHBORS)
				{
					Vec2 n = { aP.m_x + offset.m_x, aP.m_y + offset.m_y };
					if(n.m_x >= 0 && n.m_y >= 0 && n.m_x < width && n.m_y < (int32_t)m_height && IsInCave(n))
						return UINT32_MAX;
				}
			}

			const Data::Terrain* terrain = m_manifest->GetById<Data::Terrain>(m_terrainMap[i]);
			if(terrain->m_connectConversion == 0 || !m_manifest->GetById<Data::Terrain>(terrain->m_connectConversion)->m_walkable)
				return UINT32_MAX;

			// Noise makes paths less straight (and differ between structures)
			return 1 + terrain->m_connectCost + SampleNoiseMap(aP);
		};

		std::vector<uint32_t> distances((size_t)(bandSize.m_x * bandSize.m_y), UINT32_MAX);
		std::vector<int32_t> previous((size_t)(bandSize.m_x * bandSize.m_y), -1);
		std::set<std::pair<uint32_t, int32_t>> queue; // Distance and band index, ordered for the same result on all platforms

		for(int32_t y = bandMin.m_y; y <= bandMax.m_y; y++)
		{
			for(int32_t x = bandMin.m_x; x <= bandMax.m_x; x++)
			{
				if(getRoot((size_t)(x + y * width)) == aPocketRoot)
				{
					int32_t bandIndex = (x - bandMin.m_x) + (y - bandMin.m_y) * bandSize.m_x;
					distances[(size_t)bandIndex] = 0;
					queue.insert({ 0, bandIndex });
				}
			}
		}

		while(queue.size() > 0)
		{
			auto [distance, bandIndex] = *queue.begin();
			queue.erase(queue.begin());

			if(distance > distances[(size_t)bandIndex])
				continue;

			Vec2 p = { bandMin.m_x + bandIndex % bandSize.m_x, bandMin.m_y + bandIndex / bandSize.m_x };
			size_t mapIndex = (size_t)(p.m_x + p.m_y * width);

			bool passable;
			getCost(p, passable);

			if(passable)
			{
				uint32_t root = getRoot(mapIndex);
				if(root == UINT32_MAX || !aUnconnectedRoots[root])
				{
					// Reached the main area: make the path passable
					for(int32_t i = bandIndex; i != -1; i = previous[(size_t)i])
					{
						Vec2 q = { bandMin.m_x + i % bandSize.m_x, bandMin.m_y + i / bandSize.m_x };
						int32_t j = q.m_x + q.m_y * width;
						bool pathPassable;
						getCost(q, pathPassable);
						if(!pathPassable)
						{
							m_passableMap[(size_t)j] = 1;
							aOutPath.push_back(j);
						}
					}
					return true;
				}
			}

			for(const Vec2& offset : NEIGHBORS)
			{
				Vec2 n = { p.m_x + offset.m_x, p.m_y + offset.m_y };
				if(n.m_x < bandMin.m_x || n.m_y < bandMin.m_y || n.m_x > bandMax.m_x || n.m_y > bandMax.m_y)
					continue;

				bool neighborPassable;
				uint32_t cost = getCost(n, neighborPassable);
				if(cost == UINT32_MAX)
					continue;

				int32_t neighborBandIndex = (n.m_x - bandMin.m_x) + (n.m_y - bandMin.m_y) * bandSize.m_x;
				uint32_t neighborDistance = distance + cost;
				if(neighborDistance < distances[(size_t)neighborBandIndex])
				{
					distances[(size_t)neighborBandIndex] = neighborDistance;
					previous[(size_t)neighborBandIndex] = bandIndex;
					queue.insert({ neighborDistance, neighborBandIndex });
				}
			}
		}

		return false;
	}

	void			
	World::Builder::InitRoutes()
	{
		for(std::unique_ptr<Route>& route : m_routes)
		{
			switch(route->m_type)
			{
			case Route::TYPE_RANDOM:
				{
					TP_CHECK(m_playerSpawnDistanceCombined, "No player spawn distance field.");

					// Set from position to be somewhat away from player spawns
					{
						std::vector<Vec2> positions;
						m_playerSpawnDistanceCombined->GetPositionsMoreThanValue(m_playerSpawnDistanceCombined->GetMax() / 2, positions);
						route->m_from = positions[Roll(0, (uint32_t)positions.size() - 1)];
					}

					// Create distance map for from position 
					DistanceField distanceField((int32_t)m_width, (int32_t)m_height);
					distanceField.Generate({ route->m_from }, m_walkable, UINT32_MAX);

					// Find a destination that's both far away from play spawns and the origin
					{
						DistanceField combinedDistanceField(distanceField);
						combinedDistanceField.CombineMin(m_playerSpawnDistanceCombined.get());
						std::vector<Vec2> positions;
						m_playerSpawnDistanceCombined->GetPositionsMoreThanValue(m_playerSpawnDistanceCombined->GetMax() / 2, positions);
						route->m_to = positions[Roll(0, (uint32_t)positions.size() - 1)];
					}

					// Make route
					distanceField.MakePath(m_random, route->m_to, route->m_positions);
				}
				break;

			default:
				break;
			}
		}
	}

	void
	World::Builder::InitZones()
	{
		if(m_params->m_playerSpawnZoneRadius > 0)
		{
			TP_CHECK(m_playerSpawnDistanceCombined, "No player spawn distance field.");

			if(m_params->m_playerSpawnZoneId != 0)
			{
				std::vector<Vec2> positions;
				m_playerSpawnDistanceCombined->GetPositionsInRange(0, m_params->m_playerSpawnZoneRadius, positions);

				for(const Vec2& position : positions)
					UpdateZoneMap(position, m_params->m_playerSpawnZoneId);
			}

			if (m_params->m_outsidePlayerSpawnZoneId != 0)
			{
				std::vector<Vec2> positions;
				m_playerSpawnDistanceCombined->GetPositionsMoreThanValue(m_params->m_playerSpawnZoneRadius, positions);

				for (const Vec2& position : positions)
					UpdateZoneMap(position, m_params->m_outsidePlayerSpawnZoneId);
			}

			for(const PlayerSpawnZoneBoundaryEntity& playerSpawnZoneBoundaryEntity : m_playerSpawnZoneBoundaryEntities)
			{
				MapData::EntitySpawn entitySpawn;
				entitySpawn.m_entityId = playerSpawnZoneBoundaryEntity.m_entityId;
				entitySpawn.m_mapEntitySpawnId = m_params->m_objectMapEntitySpawnId;

				std::vector<Vec2> positions;
				m_playerSpawnDistanceCombined->GetPositionsInRange(
					m_params->m_playerSpawnZoneRadius + playerSpawnZoneBoundaryEntity.m_range.m_min, 
					m_params->m_playerSpawnZoneRadius + playerSpawnZoneBoundaryEntity.m_range.m_max, 
					positions);

				for (const Vec2& position : positions)
				{
					entitySpawn.m_x = position.m_x;
					entitySpawn.m_y = position.m_y;
					m_entitySpawns.push_back(entitySpawn);
				}
			}
		}
	}

	//---------------------------------------------------------------------------------

	World::IExecuteFactory::IExecuteFactory()
	{
		Register<ExecuteAll>();
		Register<ExecuteOneOf>();
		Register<ExecutePalettedMap>();
		Register<ExecuteDouble>();
		Register<ExecuteMedianFilter>();
		Register<ExecuteGrow>();
		Register<ExecuteDespeckle>();
		Register<ExecuteAddBorders>();
		Register<ExecuteDebugTerrainMap>();
		Register<ExecuteOverlay>();
		Register<ExecuteTerrainModifierMap>();
		Register<ExecuteAddEntitySpawns>();
		Register<ExecuteAddBoss>();
		Register<ExecuteLevelRange>();
		Register<ExecutePlayerSpawns>();
		Register<ExecuteAddSpecialEntitySpawn>();
		Register<ExecuteAddRandomRoute>();
		Register<ExecuteAddPlayerSpawnZoneBoundaryEntity>();
		Register<ExecuteAddStructures>();
		Register<ExecuteAddCaves>();
		Register<ExecuteAddEnemies>();
	}

	World::IExecuteFactory::~IExecuteFactory()
	{

	}

	//---------------------------------------------------------------------------------

	void				
	World::ExecuteAll::Run(
		Builder*						aBuilder) const 
	{
		for (const std::unique_ptr<IExecute>& child : m_compound.m_children)
		{
			if (aBuilder->MaybeDoIt(child->m_header.m_value))
				child->Run(aBuilder);
		}
	}

	void				
	World::ExecuteOneOf::Run(
		Builder*						aBuilder) const 
	{
		if (m_compound.m_children.size() > 0)
		{
			uint32_t totalWeightOfChildren = 0;
			for (const std::unique_ptr<IExecute>& child : m_compound.m_children)
				totalWeightOfChildren += child->m_header.m_value;

			uint32_t roll = aBuilder->Roll(1, totalWeightOfChildren);
			uint32_t sum = 0;
			const IExecute* execute = NULL;
			for (const std::unique_ptr<IExecute>& child : m_compound.m_children)
			{
				sum += child->m_header.m_value;
				if (roll <= sum)
				{
					execute = child.get();
					break;
				}
			}

			assert(execute != NULL);

			execute->Run(aBuilder);
		}
	}

	void				
	World::ExecutePalettedMap::Run(
		Builder*						aBuilder) const 
	{
		aBuilder->m_width = m_width;
		aBuilder->m_height = m_height;
		aBuilder->m_terrainMap.resize(m_tiles.size());
		for (size_t i = 0; i < aBuilder->m_terrainMap.size(); i++)
			aBuilder->m_terrainMap[i] = aBuilder->GetTerrainPaletteEntry(m_tiles[i]);
	}

	void				
	World::ExecuteDouble::Run(
		Builder*						aBuilder) const 
	{
		TP_CHECK(aBuilder->m_terrainMap.size() > 0 && aBuilder->m_width > 0 && aBuilder->m_height > 0, "No terrain map.");

		for(uint32_t k = 0; k < m_passes; k++)
		{
			uint32_t newWidth = aBuilder->m_width * 2;
			uint32_t newHeight = aBuilder->m_height * 2;
			std::vector<uint32_t> newTerrainMap;
			newTerrainMap.resize(newWidth * newHeight, 0);

			std::vector<Vec2> holes;

			for (int32_t y = 0; y < (int32_t)aBuilder->m_height; y++)
			{
				for (int32_t x = 0; x < (int32_t)aBuilder->m_width; x++)
				{
					static const Vec2 OFFSETS[4] = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } };
					uint32_t roll = aBuilder->Random2Bits();
					for (uint32_t i = 0; i < 4; i++)
					{
						const Vec2& offset = OFFSETS[i];
						Vec2 p = { 2 * x + offset.m_x, 2 * y + offset.m_y };
						if (i == roll)
							newTerrainMap[p.m_x + p.m_y * (int32_t)newWidth] = aBuilder->m_terrainMap[x + y * aBuilder->m_width];
						else
							holes.push_back(p);
					}
				}
			}

			while (holes.size() > 0)
			{
				std::vector<std::pair<int32_t, uint32_t>> changes;

				for (size_t i = 0; i < holes.size(); i++)
				{
					const Vec2& hole = holes[i];

					uint32_t candidates[4];
					uint32_t numCandidates = 0;

					static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
					for (uint32_t j = 0; j < 4; j++)
					{
						Vec2 p = { hole.m_x + NEIGHBORS[j].m_x, hole.m_y + NEIGHBORS[j].m_y };
						if (p.m_x >= 0 && p.m_y >= 0 && p.m_x < (int32_t)newWidth && p.m_y < (int32_t)newHeight)
						{
							uint32_t terrainId = newTerrainMap[p.m_x + p.m_y * (int32_t)newWidth];
							if (terrainId != 0)
								candidates[numCandidates++] = terrainId;
						}
					}

					if (numCandidates > 0)
					{
						changes.push_back({ hole.m_x + hole.m_y * (int32_t)newWidth, candidates[aBuilder->Roll(0, numCandidates - 1)] });

						Helpers::RemoveCyclicFromVector(holes, i);
						i--;
					}
				}

				assert(changes.size() > 0);

				for (const std::pair<int32_t, uint32_t>& change : changes)
					newTerrainMap[change.first] = change.second;
			}

			if (!m_noMutations)
			{
				for (uint32_t y = 1; y < newHeight - 1; y++)
				{
					for (uint32_t x = 1; x < newWidth - 1; x++)
					{
						uint32_t i = x + y * newWidth;
						const Data::Terrain* terrain = aBuilder->m_manifest->GetById<Data::Terrain>(newTerrainMap[i]);
						newTerrainMap[i] = terrain->Mutate(aBuilder->m_random);
					}
				}
			}

			aBuilder->m_terrainMap = std::move(newTerrainMap);
			aBuilder->m_width = newWidth;
			aBuilder->m_height = newHeight;
		}
	}

	void				
	World::ExecuteMedianFilter::Run(
		Builder*						aBuilder) const 
	{
		typedef std::map<uint32_t, uint32_t> Counters;
		Counters counters;

		std::vector<uint32_t> newTerrainMap;
		newTerrainMap.resize(aBuilder->m_terrainMap.size());

		int32_t radiusSquared = (int32_t)(m_radius * m_radius);

		for (int32_t y = 0; y < (int32_t)aBuilder->m_height; y++)
		{
			for (int32_t x = 0; x < (int32_t)aBuilder->m_width; x++)
			{
				counters.clear();

				int32_t minX = Base::Max(x - (int32_t)m_radius, 0);
				int32_t minY = Base::Max(y - (int32_t)m_radius, 0);
				int32_t maxX = Base::Min(x + (int32_t)m_radius, (int32_t)aBuilder->m_width - 1);
				int32_t maxY = Base::Min(y + (int32_t)m_radius, (int32_t)aBuilder->m_height - 1);

				for (int32_t iy = minY; iy <= maxY; iy++)
				{
					for (int32_t ix = minX; ix <= maxX; ix++)
					{
						Vec2 d = { ix - x, iy - y };
						if (d.m_x * d.m_x + d.m_y * d.m_y <= radiusSquared)
						{
							uint32_t terrainId = aBuilder->m_terrainMap[ix + iy * (int32_t)aBuilder->m_width];

							Counters::iterator i = counters.find(terrainId);
							if (i == counters.end())
								counters[terrainId] = 1;
							else
								i->second++;
						}
					}
				}

				assert(counters.size() > 0);
				newTerrainMap[x + y * (int32_t)aBuilder->m_width] = counters.cbegin()->first;
			}
		}

		aBuilder->m_terrainMap = std::move(newTerrainMap);
	}

	void				
	World::ExecuteGrow::Run(
		Builder*						aBuilder) const 
	{
		const Data::Terrain* terrain = aBuilder->m_manifest->GetById<Data::Terrain>(m_terrainId);

		std::unordered_map<uint32_t, uint32_t> probabilites;
		for (const Data::Terrain::GrowsInto& growsInto : terrain->m_growsInto)
			probabilites[growsInto.m_terrainId] = growsInto.m_probability;

		std::vector<uint32_t> changes;

		for (uint32_t y = 0; y < aBuilder->m_height; y++)
		{
			for (uint32_t x = 0; x < aBuilder->m_width; x++)
			{
				uint32_t offset = x + y * aBuilder->m_width;
				std::unordered_map<uint32_t, uint32_t>::const_iterator i = probabilites.find(aBuilder->m_terrainMap[offset]);
				if (i != probabilites.cend())
				{
					uint32_t probability = i->second;
					static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
					for (uint32_t j = 0; j < 4; j++)
					{
						const Vec2& neighbor = NEIGHBORS[j];
						if (aBuilder->GetTerrainIdWithOffset(x, y, neighbor.m_x, neighbor.m_y) == m_terrainId)
						{
							if (aBuilder->Roll(1, 100) <= probability)
							{
								changes.push_back(offset);
								break;
							}
						}
					}
				}
			}
		}

		for (uint32_t change : changes)
			aBuilder->m_terrainMap[change] = m_terrainId;
	}

	void				
	World::ExecuteDespeckle::Run(
		Builder*						aBuilder) const 
	{
		std::vector<std::pair<uint32_t, uint32_t>> changes;

		for (uint32_t y = 0; y < aBuilder->m_height; y++)
		{
			for (uint32_t x = 0; x < aBuilder->m_width; x++)
			{
				uint32_t offset = x + y * aBuilder->m_width;
				uint32_t terrainId = aBuilder->m_terrainMap[offset];

				if(terrainId == m_fromTerrainId || m_fromTerrainId == 0)
				{
					uint32_t replacementTerrainId = 0;

					static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };

					if(m_toTerrainId == 0)
					{
						for (uint32_t j = 0; j < 4; j++)
						{
							const Vec2& neighbor = NEIGHBORS[j];
							uint32_t neighborTerrainId = aBuilder->GetTerrainIdWithOffset(x, y, neighbor.m_x, neighbor.m_y);
							if (neighborTerrainId == terrainId || (replacementTerrainId != 0 && replacementTerrainId != neighborTerrainId))
							{
								replacementTerrainId = 0;
								break;
							}
							else if (replacementTerrainId == 0)
							{
								replacementTerrainId = neighborTerrainId;
							}
						}
					}
					else
					{
						replacementTerrainId = m_toTerrainId;

						for (uint32_t j = 0; j < 4 && replacementTerrainId != 0; j++)
						{
							const Vec2& neighbor = NEIGHBORS[j];
							uint32_t neighborTerrainId = aBuilder->GetTerrainIdWithOffset(x, y, neighbor.m_x, neighbor.m_y);
							if(neighborTerrainId == m_fromTerrainId || neighborTerrainId == 0)
								replacementTerrainId = 0;
						}

					}

					if (replacementTerrainId != 0)
						changes.push_back({ offset, replacementTerrainId });
				}
			}
		}

		for (const std::pair<uint32_t, uint32_t>& change : changes)
			aBuilder->m_terrainMap[change.first] = change.second;
	}

	void				
	World::ExecuteAddBorders::Run(
		Builder*						aBuilder) const 
	{
		const Data::Terrain* terrain = aBuilder->m_manifest->GetById<Data::Terrain>(m_terrainId);

		std::unordered_map<uint32_t, uint32_t> borders;
		for (const Data::Terrain::Border& border : terrain->m_borders)
		{
			for (uint32_t neighborTerrainId : border.m_neighborTerrain)
				borders[neighborTerrainId] = border.m_terrainId;
		}

		std::vector<std::pair<uint32_t, uint32_t>> changes;

		for (uint32_t y = 0; y < aBuilder->m_height; y++)
		{
			for (uint32_t x = 0; x < aBuilder->m_width; x++)
			{
				uint32_t offset = x + y * aBuilder->m_width;
				if (aBuilder->m_terrainMap[offset] == m_terrainId)
				{
					uint32_t candidates[4];
					uint32_t numCandidates = 0;

					static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
					for (uint32_t j = 0; j < 4; j++)
					{
						const Vec2& neighbor = NEIGHBORS[j];
						uint32_t neighborTerrainId = aBuilder->GetTerrainIdWithOffset(x, y, neighbor.m_x, neighbor.m_y);
						std::unordered_map<uint32_t, uint32_t>::const_iterator i = borders.find(neighborTerrainId);
						if (i != borders.cend())
							candidates[numCandidates++] = i->second;
					}

					if (numCandidates > 0)
						changes.push_back({ offset, candidates[aBuilder->Roll(0, numCandidates - 1)] });
				}
			}
		}

		for (const std::pair<uint32_t, uint32_t>& change : changes)
			aBuilder->m_terrainMap[change.first] = change.second;
	}

	void				
	World::ExecuteDebugTerrainMap::Run(
		Builder*						aBuilder) const 
	{
		TP_CHECK(aBuilder->m_terrainMap.size() > 0 && aBuilder->m_width > 0 && aBuilder->m_height > 0, "No terrain map.");

		std::unordered_map<uint32_t, Image::RGBA> objects;
		for (const MapData::EntitySpawn& entitySpawn : aBuilder->m_entitySpawns)
		{
			if(entitySpawn.m_mapEntitySpawnId != 0)
			{
				const Data::MapEntitySpawn* mapEntitySpawn = aBuilder->m_manifest->GetById<Data::MapEntitySpawn>(entitySpawn.m_mapEntitySpawnId);
				if(mapEntitySpawn->m_debugColor.has_value())
				{
					uint32_t offset = (uint32_t)(entitySpawn.m_x + entitySpawn.m_y * (int32_t)aBuilder->m_width);
					objects[offset] = mapEntitySpawn->m_debugColor.value();
				}
			}
		}

		Image image;
		image.Allocate(aBuilder->m_width, aBuilder->m_height);
		Image::RGBA* out = image.GetData();
		for (uint32_t i = 0; i < aBuilder->m_width * aBuilder->m_height; i++)
		{
			std::unordered_map<uint32_t, Image::RGBA>::const_iterator j = objects.find(i);
			if (j != objects.cend())
			{
				*out = j->second;
			}
			else
			{
				uint32_t terrainId = aBuilder->m_terrainMap[i];
				if (terrainId != 0)
				{
					const Data::Terrain* terrain = aBuilder->m_manifest->GetById<Data::Terrain>(terrainId);
					*out = terrain->m_debugColor;
				}
				else
				{
					*out = { 0, 0, 0, 255 };
				}
			}
			out++;
		}

		image.SavePNG(m_path.c_str());
	}

	void				
	World::ExecuteOverlay::Run(
		Builder*						aBuilder) const 
	{
		const Data::Noise* noise = aBuilder->m_manifest->GetById<Data::Noise>(m_noiseId);
		NoiseInstance noiseInstance(noise, aBuilder->m_random);

		for (int32_t y = (uint32_t)m_border; y < (int32_t)(aBuilder->m_height - m_border); y++)
		{
			for (int32_t x = (uint32_t)m_border; x < (int32_t)(aBuilder->m_width - m_border); x++)
			{
				uint32_t offset = (uint32_t)x + (uint32_t)y * aBuilder->m_width;

				int32_t sample = noiseInstance.Sample({ x, y });
				uint32_t terrainId = aBuilder->m_terrainMap[offset];

				// Terrain?
				{
					uint32_t newTerrainId = 0;

					for (const std::unique_ptr<Terrain>& overlayTerrain : m_terrains)
					{
						if (overlayTerrain->m_condition.Check(sample) && !overlayTerrain->IsExcluded(terrainId))
						{
							newTerrainId = overlayTerrain->m_terrainId;
							break;
						}
					}

					if (newTerrainId != 0)
						aBuilder->m_terrainMap[offset] = newTerrainId;
				}

				// Doodad?
				if(sample >= 0)
				{
					uint32_t s = (uint32_t)sample;

					for (const std::unique_ptr<Doodad>& overlayDoodad : m_doodads)
					{
						if(s >= overlayDoodad->m_range.m_min && s <= overlayDoodad->m_range.m_max && overlayDoodad->HasTerrainId(terrainId))
						{
							uint32_t probability = overlayDoodad->m_probability.m_min + 
								((overlayDoodad->m_probability.m_max - overlayDoodad->m_probability.m_min) * (s - overlayDoodad->m_range.m_min)) / (overlayDoodad->m_range.m_max - overlayDoodad->m_range.m_min);

							if(aBuilder->Roll(1, 100) <= probability)
							{
								Builder::Doodad doodad;
								doodad.m_doodadId = overlayDoodad->m_doodadId;
								doodad.m_position = { x, y };
								aBuilder->m_doodads.push_back(doodad);
							}
						}
					}
				}

				offset++;
			}
		}
	}

	void				
	World::ExecuteTerrainModifierMap::Run(
		Builder*						aBuilder) const 
	{
		TP_CHECK(TerrainModifier::ValidateId(m_id), "No terrain modifier specified.");
		std::vector<int32_t>& terrainModifierMap = aBuilder->m_terrainModifierMaps[m_id];
		terrainModifierMap.resize(aBuilder->m_width * aBuilder->m_height, 0);

		const Data::Noise* noise = aBuilder->m_manifest->GetById<Data::Noise>(m_noiseId);
		NoiseInstance noiseInstance(noise, aBuilder->m_random);

		uint32_t offset = 0;
		for (int32_t y = 0; y < (int32_t)aBuilder->m_height; y++)
		{
			for (int32_t x = 0; x < (int32_t)aBuilder->m_width; x++)
			{
				int32_t sample = noiseInstance.Sample({ x, y });

				terrainModifierMap[offset] = sample;

				offset++;
			}
		}

		if (!m_debug.empty())
		{
			Image image;
			image.Allocate(aBuilder->m_width, aBuilder->m_height);

			Image::RGBA* out = image.GetData();

			for (uint32_t i = 0; i < aBuilder->m_width * aBuilder->m_height; i++)
			{
				int32_t v = terrainModifierMap[i];
				if (v < 0)
					v = 0;
				else if (v > 255)
					v = 255;

				out->m_r = (uint8_t)v;
				out->m_g = (uint8_t)v;
				out->m_b = (uint8_t)v;
				out->m_a = 255;
				out++;
			}

			image.SavePNG(m_debug.c_str());
		}
	}

	void				
	World::ExecuteAddEntitySpawns::Run(
		Builder*						aBuilder) const 
	{
		TP_CHECK(m_mapEntitySpawns.size() > 0, "No entity spawns defined.");

		for (int32_t y = 1; y < (int32_t)aBuilder->m_height - 1; y++)
		{
			for (int32_t x = 1; x < (int32_t)aBuilder->m_width - 1; x++)
			{
				int32_t i = x + y * (int32_t)aBuilder->m_width;
				uint32_t terrainId = aBuilder->m_terrainMap[i];

				if(CheckTerrain(terrainId) && (!m_needWalkable || aBuilder->m_walkable.contains({ x, y })))
				{
					// No need to check bounds as we avoid the edges already
					uint32_t neighborTerrainIds[4] = 
					{
						aBuilder->m_terrainMap[i - 1],
						aBuilder->m_terrainMap[i + 1],
						aBuilder->m_terrainMap[i - (int32_t)aBuilder->m_width],
						aBuilder->m_terrainMap[i + (int32_t)aBuilder->m_width],
					};
					
					uint32_t probability = m_probability;

					for(const NeighborTerrain& neighborTerrain : m_neighborTerrains)
					{
						bool hasNeighbor = 
							neighborTerrainIds[0] == neighborTerrain.m_terrainId ||
							neighborTerrainIds[1] == neighborTerrain.m_terrainId ||
							neighborTerrainIds[2] == neighborTerrain.m_terrainId ||
							neighborTerrainIds[3] == neighborTerrain.m_terrainId;

						if(neighborTerrain.m_required)
						{
							if(!hasNeighbor)
							{
								probability = 0;
								break;
							}
						}
						else if(hasNeighbor)
						{
							probability += 	neighborTerrain.m_probabilityBonus;
						}
					}

					if(probability > 0 && aBuilder->Roll(1, 1000) <= probability)
					{
						bool canPlace = true;

						if(m_minDistanceToNearby > 0 && aBuilder->m_entitySpawnPositions.size() > 0)
						{
							// Check if we placed any other entity too close
							int32_t minX = x - (int32_t)m_minDistanceToNearby;
							int32_t maxX = x + (int32_t)m_minDistanceToNearby;
							int32_t minY = y - (int32_t)m_minDistanceToNearby;
							int32_t maxY = y + (int32_t)m_minDistanceToNearby;
							int32_t minDistanceSquared = (int32_t)(m_minDistanceToNearby * m_minDistanceToNearby);

							for(int32_t iy = minY; iy <= maxY && canPlace; iy++)
							{
								for (int32_t ix = minX; ix <= maxX && canPlace; ix++)
								{
									int32_t dx = x - ix;
									int32_t dy = y - iy;
									int32_t distanceSquared = dx * dx + dy * dy;
									if(distanceSquared <= minDistanceSquared && aBuilder->m_entitySpawnPositions.contains({ ix, iy }))
										canPlace = false;
								}
							}
						}

						if(canPlace)
						{
							MapData::EntitySpawn t;
							t.m_mapEntitySpawnId = m_mapEntitySpawns[aBuilder->Roll(0, (uint32_t)m_mapEntitySpawns.size() - 1)];
							t.m_x = x;
							t.m_y = y;
							aBuilder->m_entitySpawns.push_back(t);

							aBuilder->m_entitySpawnPositions.insert({ x, y });
						}
					}
				}
			}
		}
	}

	void				
	World::ExecuteAddBoss::Run(
		Builder*						aBuilder) const 
	{
		std::unique_ptr<Builder::Boss> t = std::make_unique<Builder::Boss>();
		t->m_tagContextId = m_tagContextId;
		t->m_influence = m_influence;
		t->m_influenceTileTransformThreshold = m_influenceTileTransformThreshold.GetRandom(aBuilder->m_random);
		t->m_mapEntitySpawnId = m_mapEntitySpawnId;
		t->m_minorBosses = m_minorBosses.get();

		for(const std::unique_ptr<Pack>& pack : m_packs)
			t->m_packs.push_back(pack.get());

		aBuilder->m_bosses.push_back(std::move(t));
	}

	void				
	World::ExecuteLevelRange::Run(
		Builder*						aBuilder) const 
	{
		aBuilder->m_levelRange = m_levelRange;
	}

	void				
	World::ExecutePlayerSpawns::Run(
		Builder*						aBuilder) const 
	{
		for(uint32_t i = 0; i < m_count; i++)
		{
			std::unique_ptr<Builder::PlayerSpawn> t = std::make_unique<Builder::PlayerSpawn>();
			aBuilder->m_playerSpawns.push_back(std::move(t));
		}
	}

	void
	World::ExecuteAddSpecialEntitySpawn::Run(
		Builder*						aBuilder) const
	{
		aBuilder->m_specialEntities.push_back(m_specialEntity);
	}

	void				
	World::ExecuteAddStructures::Run(
		Builder*						aBuilder) const
	{
		aBuilder->m_addStructures.push_back(m_structures);
	}

	void
	World::ExecuteAddCaves::Run(
		Builder*						aBuilder) const
	{
		aBuilder->m_addCaves.push_back(m_caves);
	}

	void
	World::ExecuteAddEnemies::Run(
		Builder*						aBuilder) const
	{
		aBuilder->m_addEnemies.push_back(&m_enemies);
	}

	void
	World::ExecuteAddRandomRoute::Run(
		Builder*						aBuilder) const
	{
		std::unique_ptr<Builder::Route> t = std::make_unique<Builder::Route>();
		t->m_type = Builder::Route::TYPE_RANDOM;
		t->m_routeId = m_routeId;
		aBuilder->m_routes.push_back(std::move(t));
	}

	void				
	World::ExecuteAddPlayerSpawnZoneBoundaryEntity::Run(
		Builder*						aBuilder) const
	{
		aBuilder->m_playerSpawnZoneBoundaryEntities.push_back({ m_range, m_entityId });
	}

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
				{
					_AddTerrainPaletteEntry(aChild);
				}
				else if(aChild->m_tag == "execute")
				{
					std::unique_ptr<IExecute> t(m_executeFactory.Create(IExecute::SourceToType(aChild)));
					t->FromSource(&m_executeFactory, aChild);
					m_executes.m_children.push_back(std::move(t));
				}
				else if(aChild->m_name == "player_spawn_entity")
				{
					m_params.m_playerSpawnEntityId = aChild->GetId(DataType::ID_ENTITY);
				}
				else if (aChild->m_name == "object_map_entity_spawn")
				{
					m_params.m_objectMapEntitySpawnId = aChild->GetId(DataType::ID_MAP_ENTITY_SPAWN);
				}
				else if (aChild->m_name == "connect_conversion_radius")
				{
					aChild->GetUIntArray(m_params.m_connectConversionRadius);
				}
				else if (aChild->m_name == "player_spawn_zone_radius")
				{
					m_params.m_playerSpawnZoneRadius = aChild->GetUInt32();
				}
				else if (aChild->m_name == "player_spawn_zone")
				{
					m_params.m_playerSpawnZoneId = aChild->GetId(DataType::ID_ZONE);
				}
				else if (aChild->m_name == "outside_player_spawn_zone")
				{
					m_params.m_outsidePlayerSpawnZoneId = aChild->GetId(DataType::ID_ZONE);
				}
				else
				{
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				}
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

		m_executes.ToStream(aWriter);
		m_params.ToStream(aWriter);
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

		if(!m_executes.FromStream(&m_executeFactory, aReader))
			return false;
		if(!m_params.FromStream(aReader))
			return false;
		return true;
	}

	bool		
	World::Build(
		const Manifest*					aManifest,
		MapGeneratorRuntime*			aMapGeneratorRuntime,
		uint32_t						aSeed,
		const MapData*					aSourceMapData,
		const char*						aDebugImagePath,
		std::unique_ptr<MapData>&		aOutMapData) const 
	{
		Builder builder;
		builder.m_terrainPalette = &m_terrainPalette;
		builder.m_params = &m_params;
		builder.m_manifest = aManifest;
		builder.m_mapGeneratorRuntime = aMapGeneratorRuntime;
		builder.m_random.seed(aSeed);

		for(const std::unique_ptr<IExecute>& execute : m_executes.m_children)
			execute->Run(&builder);

		builder.IdentifyWalkableAreas();

		if(builder.m_walkableAreas.size() == 0)
			return false; // This should be very unlikely

		builder.InitNoiseMap();
		builder.ConnectWalkableAreas();
		builder.InitCaves();
		builder.InitBosses();
		builder.InitPlayerSpawns();
		builder.InitEnemies();
		builder.InitSpecialEntities();
		builder.InitStructures();
		builder.InitRoutes();
		builder.InitZones();

		//{
		//	ExecuteDebugTerrainMap debugTerrainMap;
		//	debugTerrainMap.m_path = "debug-terrain-map.png";
		//	debugTerrainMap.Run(&builder);
		//}

		builder.CreateMapData(aSourceMapData, aOutMapData);

		if(aDebugImagePath != NULL && aDebugImagePath[0] != '\0')
			aOutMapData->WriteDebugTileMapPNG(aManifest, aDebugImagePath);

		return true;
	}

	void
	World::Validate(
		const Manifest*					aManifest,
		const char*						aMapName,
		std::unordered_set<std::string>& aOutErrors) const
	{
		std::function<void(const CompoundExecute&)> validateCompound = [&](
			const CompoundExecute&		aCompound)
		{
			for(const std::unique_ptr<IExecute>& execute : aCompound.m_children)
			{
				switch(execute->m_header.m_type)
				{
				case IExecute::TYPE_ALL:
					validateCompound(((const ExecuteAll*)execute.get())->m_compound);
					break;

				case IExecute::TYPE_ONE_OF:
					validateCompound(((const ExecuteOneOf*)execute.get())->m_compound);
					break;

				case IExecute::TYPE_ADD_STRUCTURES:
					{
						const Structures& structures = ((const ExecuteAddStructures*)execute.get())->m_structures;

						for(uint32_t mapSegmentId : structures.m_mapSegmentIds)
						{
							const Data::MapSegment* mapSegment = aManifest->GetById<Data::MapSegment>(mapSegmentId);
							if(!mapSegment->m_tileMap)
								aOutErrors.insert(Helpers::Format("Map '%s' places structure '%s', which has no paletted map.", aMapName, mapSegment->m_name.c_str()));
							else if(mapSegment->m_entrances.empty())
								aOutErrors.insert(Helpers::Format("Map '%s' places structure '%s', which has no entrances.", aMapName, mapSegment->m_name.c_str()));

							if(mapSegment->m_tileMap)
							{
								for(const Data::MapSegment::Object& object : mapSegment->m_tileMap->m_objects)
								{
									if(object.m_type != Data::MapSegment::OBJECT_TYPE_ENTITY_SPAWN && object.m_type != Data::MapSegment::OBJECT_TYPE_PORTAL)
										aOutErrors.insert(Helpers::Format("Map '%s' places structure '%s', which has objects that can't be used in a generated world.", aMapName, mapSegment->m_name.c_str()));
								}
							}
						}

						for(const Structures::AnchorSpawn& anchorSpawn : structures.m_anchorSpawns)
						{
							bool found = false;
							for(uint32_t mapSegmentId : structures.m_mapSegmentIds)
							{
								if(aManifest->GetById<Data::MapSegment>(mapSegmentId)->HasAnchor(anchorSpawn.m_anchor))
								{
									found = true;
									break;
								}
							}

							if(!found)
								aOutErrors.insert(Helpers::Format("Map '%s' has a spawn for anchor '%s', but none of its structures have that anchor.", aMapName, anchorSpawn.m_anchor.c_str()));
						}
					}
					break;

				case IExecute::TYPE_ADD_CAVES:
					{
						const Caves& caves = ((const ExecuteAddCaves*)execute.get())->m_caves;

						if(!aManifest->GetById<Data::Terrain>(caves.m_floorTerrainId)->m_walkable)
							aOutErrors.insert(Helpers::Format("Map '%s' has caves with a floor terrain that isn't walkable.", aMapName));

						bool hasCaveEntrance = false;
						for(uint32_t terrainId : caves.m_terrainIds)
						{
							for(const std::unique_ptr<Data::Terrain::Tile>& tile : aManifest->GetById<Data::Terrain>(terrainId)->m_tiles)
							{
								if(tile->m_caveEntranceSpriteId != 0)
									hasCaveEntrance = true;
							}
						}

						if(!hasCaveEntrance)
							aOutErrors.insert(Helpers::Format("Map '%s' has caves, but no tile of the terrain to dig into has a 'cave_entrance'.", aMapName));
					}
					break;

				default:
					break;
				}
			}
		};

		validateCompound(m_executes);
	}

	//---------------------------------------------------------------------------------

	void		
	World::_AddTerrainPaletteEntry(
		const SourceNode*				aSource)
	{
		TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing terrain annotation.");
		uint32_t terrainId = aSource->m_annotation->GetId(DataType::ID_TERRAIN);
		char symbol = aSource->GetCharacter();
		TP_VERIFY(!m_terrainPalette.contains(symbol), aSource->m_debugInfo, "'%c' is already defined in terrain palette.");
		m_terrainPalette[symbol] = terrainId;
	}

}

