#include "../Pcheader.h"

#include <tpublic/Components/CombatPublic.h>

#include <tpublic/Data/Doodad.h>
#include <tpublic/Data/Faction.h>
#include <tpublic/Data/MapEntitySpawn.h>
#include <tpublic/Data/Noise.h>
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
			{
				const LevelMapPoint& levelMapPoint = m_levelMap[i];
				if(levelMapPoint.m_boss != NULL && levelMapPoint.m_influence > levelMapPoint.m_boss->m_influenceTileTransformThreshold && levelMapPoint.m_boss->m_factionId)
				{
					const Data::Faction* faction = m_manifest->GetById<Data::Faction>(levelMapPoint.m_boss->m_factionId);
					uint32_t toTileSpriteId = faction->GetInfluenceTileTransform(tileSpriteId);
					if(toTileSpriteId != 0)
						tileSpriteId = toTileSpriteId;
				}

				aOutMapData->m_tileMap[i] = tileSpriteId;
			}
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

			aOutMapData->m_worldInfoMap = std::make_unique<WorldInfoMap>();
			aOutMapData->m_worldInfoMap->Build(m_manifest, (int32_t)m_width, (int32_t)m_height, &levels[0], NULL, NULL, NULL);
		}

		// Doodads
		{
			std::unordered_map<Vec2, uint32_t, Vec2::Hasher> coverageMap;
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
		{
			std::unordered_set<Vec2, Vec2::Hasher> enemyPositions;

			struct EnemyTypeKey
			{
				struct Hasher
				{
					uint32_t
					operator()(
						const EnemyTypeKey& aKey) const
					{
						uint64_t h = Hash::Splitmix_2_32(aKey.m_level, aKey.m_tagContextId);
						return (uint32_t)(h ^ (aKey.m_elite ? 1ULL : 0ULL));
					}
				};

				bool
				operator ==(
					const EnemyTypeKey& aOther) const
				{
					return m_tagContextId == aOther.m_tagContextId && m_level == aOther.m_level && m_elite == aOther.m_elite;
				}

				// Public data
				uint32_t		m_tagContextId = 0;
				uint32_t		m_level = 0;
				bool			m_elite = false;
			};

			typedef std::unordered_map<EnemyTypeKey, uint32_t, EnemyTypeKey::Hasher> EnemyTypeTable;
			EnemyTypeTable enemyTypeTable;

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
								if (Roll(1, 1000) <= pack->m_probability)
								{
									// Check if anything is too close
									bool canPlace = true;

									int32_t minX = x - (int32_t)pack->m_minDistanceToNearBy;
									int32_t maxX = x + (int32_t)pack->m_minDistanceToNearBy;
									int32_t minY = y - (int32_t)pack->m_minDistanceToNearBy;
									int32_t maxY = y + (int32_t)pack->m_minDistanceToNearBy;
									int32_t minDistanceSquared = (int32_t)(pack->m_minDistanceToNearBy * pack->m_minDistanceToNearBy);

									for (int32_t iy = minY; iy <= maxY && canPlace; iy++)
									{
										for (int32_t ix = minX; ix <= maxX && canPlace; ix++)
										{
											int32_t dx = x - ix;
											int32_t dy = y - iy;
											int32_t distanceSquared = dx * dx + dy * dy;
											if (distanceSquared <= minDistanceSquared && enemyPositions.contains({ ix, iy }))
												canPlace = false;
										}
									}

									if (canPlace)
									{
										pickedPack = pack;
										break;
									}
								}
							}
						}

						if (pickedPack != NULL)
						{
							uint32_t level = point.m_level;

							bool firstEnemy = true;

							for (uint32_t tagContextId : pickedPack->m_tagContextIds)
							{
								uint32_t entityId = 0;

								{
									EnemyTypeKey enemyTypeKey;
									enemyTypeKey.m_tagContextId = tagContextId;
									enemyTypeKey.m_level = level;
									enemyTypeKey.m_elite = pickedPack->m_elite;
									EnemyTypeTable::const_iterator i = enemyTypeTable.find(enemyTypeKey);
									if(i != enemyTypeTable.cend())
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
											if (combatPublic->m_level == level && combatPublic->m_factionId == point.m_boss->m_factionId && combatPublic->IsElite() == pickedPack->m_elite)
												return true;
											return false;
										});

										TP_CHECK(picked != NULL, "Unable to pick enemy.");
										entityId = picked->m_id;

										enemyTypeTable[enemyTypeKey] = entityId;
									}
								}

								std::optional<Vec2> position;

								if (firstEnemy)
								{
									firstEnemy = false;
									position = Vec2{x, y};
								}
								else
								{
									static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
									size_t randomOffset = (size_t)m_random() % 4;
									for (size_t i = 0; i < 4; i++)
									{
										size_t j = (i + randomOffset) % 8;
										Vec2 p = { x + NEIGHBORS[j].m_x, y + NEIGHBORS[j].m_y };
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
									entitySpawn.m_mapEntitySpawnId = pickedPack->m_mapEntitySpawnId;
									entitySpawn.m_x = position->m_x;
									entitySpawn.m_y = position->m_y;
									m_entitySpawns.push_back(entitySpawn);

									m_entitySpawnPositions.insert(position.value());

									enemyPositions.insert(position.value());
								}
							}

							if(pickedPack->m_randomObject.has_value() && Roll(1, 1000) <= pickedPack->m_randomObject->m_probability)
							{
								static const Vec2 NEIGHBORS[8] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 }, { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
								size_t randomOffset = (size_t)m_random() % 8;
								std::optional<Vec2> position;

								for (size_t i = 0; i < 8; i++)
								{
									size_t j = (i + randomOffset) % 8;

									Vec2 p = { x + NEIGHBORS[j].m_x, y + NEIGHBORS[j].m_y };
									if (!m_entitySpawnPositions.contains(p) && m_walkable.contains(p))
									{
										position = p;
										break;
									}
								}

								if(position.has_value())
								{
									MapData::EntitySpawn entitySpawn;
									entitySpawn.m_mapEntitySpawnId = pickedPack->m_randomObject->m_mapEntitySpawnId;
									entitySpawn.m_x = position->m_x;
									entitySpawn.m_y = position->m_y;
									m_entitySpawns.push_back(entitySpawn);

									m_entitySpawnPositions.insert(position.value());
								}
							}
						}
					}
				}
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
				m_playerSpawns[0]->m_position = *m_walkable.cbegin();

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
	World::Builder::InitSpecialEntities()
	{
		assert(m_playerSpawns.size() > 0);

		for(const SpecialEntity& specialEntity : m_specialEntities)
		{
			TP_CHECK(specialEntity.m_mapEntitySpawns.size() > 0, "No map entity spawns defined.");
			uint32_t mapEntitySpawnId = specialEntity.m_mapEntitySpawns[Roll(0, (uint32_t)specialEntity.m_mapEntitySpawns.size() - 1)];
			Vec2 position = *m_walkable.cbegin();

			switch(specialEntity.m_placement)
			{
			case SpecialEntity::PLACEMENT_FAR_AWAY_FROM_PLAYER_SPAWNS:
				{
					TP_CHECK(m_playerSpawnDistanceCombined, "No player spawn distance field.");
					std::vector<Vec2> positions;
					m_playerSpawnDistanceCombined->GetPositionsMoreThanValue(m_playerSpawnDistanceCombined->GetMax() - 8, positions);
					if (positions.size() > 0)
						position = positions[Roll(0, (uint32_t)positions.size() - 1)];
				}
				break;

			default:
				break;
			}

			MapData::EntitySpawn t;
			t.m_mapEntitySpawnId = mapEntitySpawnId;
			t.m_x = position.m_x;
			t.m_y = position.m_y;
			m_entitySpawns.push_back(t);
			m_entitySpawnPositions.insert({ t.m_x, t.m_y });
		}
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
				uint32_t surroundingTerrainId = 0;

				static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
				for (uint32_t j = 0; j < 4; j++)
				{
					const Vec2& neighbor = NEIGHBORS[j];
					uint32_t neighborTerrainId = aBuilder->GetTerrainIdWithOffset(x, y, neighbor.m_x, neighbor.m_y);
					if (neighborTerrainId == terrainId || (surroundingTerrainId != 0 && surroundingTerrainId != neighborTerrainId))
					{
						surroundingTerrainId = 0;
						break;
					}
					else if (surroundingTerrainId == 0)
					{
						surroundingTerrainId = neighborTerrainId;
					}
				}

				if (surroundingTerrainId != 0)
				{
					changes.push_back({ offset, surroundingTerrainId });
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

		std::unordered_set<uint32_t> entitySpawnOffsets;
		for (const MapData::EntitySpawn& entitySpawn : aBuilder->m_entitySpawns)
			entitySpawnOffsets.insert((uint32_t)(entitySpawn.m_x + entitySpawn.m_y * (int32_t)aBuilder->m_width));

		Image image;
		image.Allocate(aBuilder->m_width, aBuilder->m_height);
		Image::RGBA* out = image.GetData();
		for (uint32_t i = 0; i < aBuilder->m_width * aBuilder->m_height; i++)
		{
			if (entitySpawnOffsets.contains(i))
			{
				*out = { 255, 0, 0, 255 };
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

		uint32_t offset = 0;
		for (int32_t y = 0; y < (int32_t)aBuilder->m_height; y++)
		{
			for (int32_t x = 0; x < (int32_t)aBuilder->m_width; x++)
			{
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
	World::ExecuteAddRandomRoute::Run(
		Builder*						aBuilder) const
	{
		std::unique_ptr<Builder::Route> t = std::make_unique<Builder::Route>();
		t->m_type = Builder::Route::TYPE_RANDOM;
		t->m_routeId = m_routeId;
		aBuilder->m_routes.push_back(std::move(t));
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
		builder.InitBosses();
		builder.InitPlayerSpawns();
		builder.InitSpecialEntities();
		builder.InitRoutes();

		builder.CreateMapData(aSourceMapData, aOutMapData);

		if(aDebugImagePath != NULL && aDebugImagePath[0] != '\0')
			aOutMapData->WriteDebugTileMapPNG(aManifest, aDebugImagePath);

		return true;
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

