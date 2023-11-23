#include "Pcheader.h"

#include <tpublic/BBox.h>
#include <tpublic/Image.h>
#include <tpublic/Manifest.h>
#include <tpublic/MapSegmentInstance.h>

namespace tpublic
{

	void		
	MapSegmentInstance::Generate(
		const Manifest*				aManifest,
		const Data::MapSegment*		aMapSegment,
		std::mt19937&				aRandom)
	{
		m_mapSegment = aMapSegment;

		if(aMapSegment->m_tileMap)
		{
			// Already has a tile map
			m_tileMap = &aMapSegment->m_tileMap->m_tiles[0];			
			m_size = aMapSegment->m_tileMap->m_size;

			for (const Data::MapSegment::Object& object : aMapSegment->m_tileMap->m_objects)
			{
				switch(object.m_type)
				{
				case Data::MapSegment::OBJECT_TYPE_CONNECTOR:
					_GetOrCreateConnector(object.m_id)->m_positions.push_back(object.m_position);
					break;

				default:
					m_objects.push_back(object);
					break;
				}
			}
		}
		else
		{
			// We need to generate a tile map
			switch(aMapSegment->m_type)
			{
			case Data::MapSegment::TYPE_RANDOM_ROOM:
				_GenerateRandomRoom(aManifest, aRandom);
				break;

			default:
				
				TP_CHECK(false, "Invalid map segment type.");
				break;
			}

			assert(m_size.m_x > 0 && m_size.m_y > 0);
			assert(m_generatedTileMap.size() == (size_t)(m_size.m_x * m_size.m_y));
			m_tileMap = &m_generatedTileMap[0];

			// Apply tile map modifiers
			for(const std::unique_ptr<Data::MapSegment::TileMapModifier>& tileMapModifier : aMapSegment->m_tileMapModifiers)
				tileMapModifier->Apply(&m_generatedTileMap[0], m_size, aRandom);
		}
	}

	void		
	MapSegmentInstance::CreateDebugImage(
		const Manifest*				aManifest,
		Image&						aOut) const
	{
		aOut.Allocate((uint32_t)m_size.m_x, (uint32_t)m_size.m_y);

		// Tile map
		{
			Image::RGBA* out = aOut.GetData();
			const uint32_t* in = m_tileMap;
			for (int32_t y = 0; y < m_size.m_y; y++)
			{
				for (int32_t x = 0; x < m_size.m_x; x++)
				{
					if(*in != 0)
					{
						const Data::Sprite* sprite = aManifest->m_sprites.GetById(*in);

						if(sprite->m_info.m_flags & SpriteInfo::FLAG_TILE_WALKABLE)
							*out = { 64, 64, 64, 255 };
						else
							*out = { 128, 128, 128, 255 };
					}
					else
					{
						*out = { 0, 0, 0, 0 };
					}

					in++;
					out++;
				}
			}
		}

		// Connectors
		{
			Image::RGBA* out = aOut.GetData();
			for(const std::unique_ptr<Connector>& connector : m_connectors)
			{	
				for(const Vec2& position : connector->m_positions)
					out[position.m_x + position.m_y * m_size.m_x] = { 192, 192, 0, 255 };
			}
		}
	}

	const MapSegmentInstance::Connector* 
	MapSegmentInstance::GetConnector(
		uint32_t					aMapSegmentConnectorId) const
	{
		for (const std::unique_ptr<Connector>& connector : m_connectors)
		{
			if (connector->m_mapSegmentConnectorId == aMapSegmentConnectorId)
				return connector.get();
		}
		return NULL;
	}

	//--------------------------------------------------------------------------------

	MapSegmentInstance::Connector* 
	MapSegmentInstance::_GetOrCreateConnector(
		uint32_t					aMapSegmentConnectorId)
	{
		for (std::unique_ptr<Connector>& connector : m_connectors)
		{
			if(connector->m_mapSegmentConnectorId == aMapSegmentConnectorId)
				return connector.get();
		}

		Connector* t = new Connector();
		t->m_mapSegmentConnectorId = aMapSegmentConnectorId;
		m_connectors.push_back(std::unique_ptr<Connector>(t));
		return t;
	}

	//--------------------------------------------------------------------------------

	void				
	MapSegmentInstance::_GenerateRandomRoom(
		const Manifest*				/*aManifest*/,
		std::mt19937&				aRandom)
	{
		const Data::MapSegment::RandomRoom* randomRoom = m_mapSegment->m_randomRoom.get();
		assert(randomRoom != NULL);
		assert(randomRoom->m_floorTiles.size() > 0);
		assert(randomRoom->m_wallTiles.size() > 0);		
		assert(randomRoom->m_connectors.size() > 0);

		uint32_t connectorDirections[Data::MapSegment::NUM_DIRECTIONS] = { 0 };

		for(const Data::MapSegment::RandomRoomConnector& randomRoomConnector : randomRoom->m_connectors)
			connectorDirections[randomRoomConnector.m_direction]++;

		std::vector<BBox> rects;
		uint32_t numRects = randomRoom->m_rectCount.GetRandom(aRandom);
		BBox combinedBBox;

		while(rects.size() < (size_t)numRects)
		{
			int32_t width = (int32_t)randomRoom->m_rectWidth.GetRandom(aRandom);
			int32_t height = (int32_t)randomRoom->m_rectHeight.GetRandom(aRandom);

			BBox addRange = { combinedBBox.m_min - Vec2{ width, height }, combinedBBox.m_max };
			Vec2 position;

			{
				std::uniform_int_distribution<int32_t> distribution(addRange.m_min.m_x, addRange.m_max.m_x);
				position.m_x = distribution(aRandom);
			}

			{
				std::uniform_int_distribution<int32_t> distribution(addRange.m_min.m_y, addRange.m_max.m_y);
				position.m_y = distribution(aRandom);
			}

			BBox rect = { position, position + Vec2{ width, height } };

			bool shouldAdd = false;

			if(rects.size() == 0)
			{	
				shouldAdd = true;
			}
			else
			{
				for(const BBox& existingRect : rects)
				{
					if(existingRect.CheckOverlap(rect))
					{
						shouldAdd = true;
						break;
					}
				}
			}

			if(shouldAdd)
			{
				rects.push_back(rect);
				combinedBBox = combinedBBox.Combine(rect);
			}
		}

		combinedBBox.m_min.m_x--;
		combinedBBox.m_min.m_y--;
		combinedBBox.m_max.m_x += 2; // Make space for connectors
		combinedBBox.m_max.m_y += 2;

		m_size = combinedBBox.GetSize();
		assert(m_size.m_x > 0 && m_size.m_y > 0);
		m_generatedTileMap.clear();
		m_generatedTileMap.resize(m_size.m_x * m_size.m_y, 0);

		for (const BBox& rect : rects)
		{
			for(int32_t y = rect.m_min.m_y; y < rect.m_max.m_y; y++)
			{
				for (int32_t x = rect.m_min.m_x; x < rect.m_max.m_x; x++)
				{
					int32_t ix = x - combinedBBox.m_min.m_x;
					int32_t iy = y - combinedBBox.m_min.m_y;
					int32_t i = ix + iy * m_size.m_x;
					assert((size_t)i < m_generatedTileMap.size());
					m_generatedTileMap[i] = randomRoom->m_floorTiles[0];
				}
			}
		}

		{
			uint32_t i = 0;
			std::vector<Vec2> wallPositions;

			for (int32_t y = 0; y < m_size.m_y; y++)
			{
				int32_t minY = Base::Max(y - 1, 0);
				int32_t maxY = Base::Min(y + 1, m_size.m_y - 1);

				for (int32_t x = 0; x < m_size.m_x; x++)
				{
					int32_t minX = Base::Max(x - 1, 0);
					int32_t maxX = Base::Min(x + 1, m_size.m_x - 1);

					if(m_generatedTileMap[i] == 0)
					{
						bool adjacent = false;
						for(int32_t iy = minY; iy <= maxY && !adjacent; iy++)
						{
							for (int32_t ix = minX; ix <= maxX && !adjacent; ix++)
							{
								if(m_generatedTileMap[ix + iy * m_size.m_x] != 0)
									adjacent = true;
							}
						}

						if(adjacent)
							wallPositions.push_back(Vec2{ x, y });
					}

					i++;
				}
			}

			uint32_t defaultWallTile = randomRoom->m_wallTiles[0];
			for(const Vec2& wallPosition : wallPositions)
				m_generatedTileMap[wallPosition.m_x + wallPosition.m_y * m_size.m_x] = defaultWallTile;

			uint32_t defaultFloorTile = randomRoom->m_floorTiles[0];

			std::unordered_set<Vec2, Vec2::Hasher> possibleConnectorPositions[Data::MapSegment::NUM_DIRECTIONS];
			std::vector<Vec2> possibleConnectorArray[Data::MapSegment::NUM_DIRECTIONS];

			bool hasEastConnectors = connectorDirections[Data::MapSegment::DIRECTION_EAST] > 0;
			bool hasNorthConnectors = connectorDirections[Data::MapSegment::DIRECTION_NORTH] > 0;
			bool hasSouthConnectors = connectorDirections[Data::MapSegment::DIRECTION_SOUTH] > 0;
			bool hasWestConnectors = connectorDirections[Data::MapSegment::DIRECTION_WEST] > 0;

			for (const Vec2& wallPosition : wallPositions)
			{
				uint32_t north = _SampleGeneratedTileMap(wallPosition + Vec2{ 0, -1 });
				uint32_t south = _SampleGeneratedTileMap(wallPosition + Vec2{ 0, 1 });
				uint32_t west = _SampleGeneratedTileMap(wallPosition + Vec2{ -1, 0 });
				uint32_t east = _SampleGeneratedTileMap(wallPosition + Vec2{ 1, 0 });

				if(hasEastConnectors || hasWestConnectors)
				{
					if(north == defaultWallTile && south == defaultWallTile)
					{
						if (hasEastConnectors && west == defaultFloorTile && east == 0)
						{
							possibleConnectorPositions[Data::MapSegment::DIRECTION_EAST].insert(wallPosition);
							possibleConnectorArray[Data::MapSegment::DIRECTION_EAST].push_back(wallPosition);
						}
						else if (hasWestConnectors && west == 0 && east == defaultFloorTile)
						{
							possibleConnectorPositions[Data::MapSegment::DIRECTION_WEST].insert(wallPosition);
							possibleConnectorArray[Data::MapSegment::DIRECTION_WEST].push_back(wallPosition);
						}
					}
				}

				if (hasSouthConnectors || hasNorthConnectors)
				{
					if(east == defaultWallTile && west == defaultWallTile)
					{
						if (hasSouthConnectors && north == defaultFloorTile && south == 0)
						{
							possibleConnectorPositions[Data::MapSegment::DIRECTION_SOUTH].insert(wallPosition);
							possibleConnectorArray[Data::MapSegment::DIRECTION_SOUTH].push_back(wallPosition);
						}
						else if (hasNorthConnectors && north == 0 && south == defaultFloorTile)
						{
							possibleConnectorPositions[Data::MapSegment::DIRECTION_NORTH].insert(wallPosition);
							possibleConnectorArray[Data::MapSegment::DIRECTION_NORTH].push_back(wallPosition);
						}
					}
				}
			}

			uint32_t numConnectors = randomRoom->m_connectorCount.GetRandom(aRandom);
			std::vector<Data::MapSegment::RandomRoomConnector> candidates = randomRoom->m_connectors;

			while(m_connectors.size() < (size_t)numConnectors && candidates.size() > 0)
			{
				size_t randomIndex;
				
				{
					std::uniform_int_distribution<size_t> distribution(0, candidates.size() - 1);
					randomIndex = distribution(aRandom);
				}

				Data::MapSegment::RandomRoomConnector randomRoomConnector = candidates[randomIndex];
				candidates.erase(candidates.begin() + randomIndex);

				std::vector<Vec2>& positions = possibleConnectorArray[randomRoomConnector.m_direction];

				std::optional<Vec2> selectedPosition;

				if(positions.size() > 0)
				{
					for (;;)
					{
						std::uniform_int_distribution<size_t> distribution(0, positions.size() - 1);
						Vec2 position = positions[distribution(aRandom)];

						m_generatedTileMap[position.m_x + position.m_y * m_size.m_x] = defaultFloorTile;

						if(randomRoomConnector.m_direction == Data::MapSegment::DIRECTION_SOUTH)
						{
							position.m_y++;
							m_generatedTileMap[position.m_x + position.m_y * m_size.m_x] = defaultFloorTile;
						}
						else if (randomRoomConnector.m_direction == Data::MapSegment::DIRECTION_EAST)
						{
							position.m_x++;
							m_generatedTileMap[position.m_x + position.m_y * m_size.m_x] = defaultFloorTile;
						}

						// FIXME: support connectors larger than 1x1

						selectedPosition = position;
						break;
					}

				}

				if(selectedPosition.has_value())
				{
					_GetOrCreateConnector(randomRoomConnector.m_mapSegmentConnectorId)->m_positions.push_back(selectedPosition.value());
				}
			}
		}	
	}

	uint32_t			
	MapSegmentInstance::_SampleGeneratedTileMap(
		const Vec2&			aPosition) const
	{
		if(aPosition.m_x < 0 || aPosition.m_x >= m_size.m_x || aPosition.m_y < 0 || aPosition.m_y >= m_size.m_y)
			return 0;
		return m_generatedTileMap[aPosition.m_x + aPosition.m_y * m_size.m_x];
	}

	bool				
	 MapSegmentInstance::_HasConnectorAtPositio(
		const Vec2&			aPosition) const
	{
		for(const std::unique_ptr<Connector>& connector : m_connectors)
		{
			for(const Vec2& position : connector->m_positions)
			{
				if(position == aPosition)
					return true;
			}
		}
		return false;
	}


}

