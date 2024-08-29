#include "../Pcheader.h"

#include <tpublic/MapGenerators/Dungeon.h>

#include <tpublic/Image.h>
#include <tpublic/Manifest.h>
#include <tpublic/MapData.h>

namespace tpublic::MapGenerators
{

	Dungeon::Dungeon()
		: MapGeneratorBase(ID)
	{

	}
	
	Dungeon::~Dungeon()
	{

	}

	//---------------------------------------------------------------------------------

	void		
	Dungeon::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if(aChild->m_tag == "room")
				{
					m_rooms.push_back(std::make_unique<Room>(aChild));
					TP_VERIFY(!m_roomNameTable.contains(aChild->m_name), aChild->m_debugInfo, "Room name already used.");
					m_roomNameTable[aChild->m_name] = (uint32_t)m_rooms.size() - 1;
				}
				else
				{
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				}
			}
		});

		// Resolve room names to indices
		for(std::unique_ptr<Room>& room : m_rooms)
		{
			for(RoomConnection& roomConnection : room->m_connections)
			{
				assert(roomConnection.m_roomIndex == UINT32_MAX);
				RoomNameTable::const_iterator i = m_roomNameTable.find(roomConnection.m_roomName);
				TP_VERIFY(i != m_roomNameTable.cend(), aSource->m_debugInfo, "'%s' is not a valid room name.", roomConnection.m_roomName.c_str());
				roomConnection.m_roomIndex = i->second;
			}
		}
	}
	
	void		
	Dungeon::ToStream(
		IWriter*						aWriter) const
	{
		ToStreamBase(aWriter);

		aWriter->WriteObjectPointers(m_rooms);
	}
	
	bool		
	Dungeon::FromStream(
		IReader*						aReader) 
	{
		if(!FromStreamBase(aReader))
			return false;

		if(!aReader->ReadObjectPointers(m_rooms))
			return false;
		return true;
	}

	bool		
	Dungeon::Build(
		const Manifest*					aManifest,
		MapGeneratorRuntime*			/*aMapGeneratorRuntime*/,
		uint32_t						aSeed,
		const MapData*					aSourceMapData,
		const char*						aDebugImagePath,
		std::unique_ptr<MapData>&		aOutMapData) const 
	{
		Builder builder(aManifest, aSeed, aSourceMapData, m_rooms);

		builder.GenerateRootRoom();

		while(builder.HasOpenConnectors())
			builder.GenerateAndAttachRoom();

		if (aDebugImagePath != NULL)
			builder.DebugImageOutput(aDebugImagePath);

		aOutMapData.reset(builder.CreateMapData());
		return true;
	}

	//---------------------------------------------------------------------------------

	void
	Dungeon::Builder::GenerateRootRoom()
	{
		const Room* rootRoom = NULL;

		{
			std::vector<const Room*> possibleRootRooms;

			for (const Room* room : m_sourceRooms)
			{
				if (room->m_flags & ROOM_FLAG_ROOT)
					possibleRootRooms.push_back(room);
			}

			TP_CHECK(possibleRootRooms.size() > 0, "No root rooms defined.");

			if (possibleRootRooms.size() == 1)
			{
				rootRoom = possibleRootRooms[0];
			}
			else
			{
				tpublic::UniformDistribution<size_t> distribution(0, possibleRootRooms.size() - 1);
				rootRoom = possibleRootRooms[distribution(m_random)];
			}
		}

		std::unique_ptr<GeneratedRoom> generatedRootRoom(_GenerateRoom(rootRoom));

		for(size_t i = 0; i < generatedRootRoom->m_connectors.size(); i++)
		{
			OpenConnector t;
			t.m_connectorIndex = i;
			t.m_generatedRoom = generatedRootRoom.get();
			m_openConnectors.push_back(t);
		}

		_AddGeneratedRoom(generatedRootRoom);
	}

	void
	Dungeon::Builder::GenerateAndAttachRoom()
	{
		assert(!m_openConnectors.empty());

		//printf("[generate and attach room: %zu open connectors]\n", m_openConnectors.size());

		//for(const OpenConnector& x : m_openConnectors)
		//{
		//	const GeneratedRoomConnector* y = x.m_generatedRoom->m_connectors[x.m_connectorIndex].get();
		//	for(const Vec2& pos : y->m_connector->m_positions)
		//	{
		//		Vec2 s = pos + x.m_generatedRoom->m_min;
		//		printf("<%d %d>", s.m_x, s.m_y);
		//	}

		//	printf(" %s\n", x.m_generatedRoom->m_mapSegmentInstance.m_mapSegment->m_name.c_str());
		//}

		// Pick random open connector
		OpenConnector openConnector;

		{
			size_t i = 0;
			
			if (m_openConnectors.size() > 1)
			{
				tpublic::UniformDistribution<size_t> distribution(0, m_openConnectors.size() - 1);
				i = distribution(m_random);
			}

			openConnector = m_openConnectors[i];
			m_openConnectors.erase(m_openConnectors.begin() + i);
		}

		assert(openConnector.m_generatedRoom != NULL && openConnector.m_connectorIndex < openConnector.m_generatedRoom->m_connectors.size());
		const GeneratedRoomConnector* generatedRoomConnector = openConnector.m_generatedRoom->m_connectors[openConnector.m_connectorIndex].get();
		
		std::vector<const RoomConnection*> candidates;
		uint32_t totalWeight = 0;

		for(const RoomConnection* t : generatedRoomConnector->m_possibleConnections)
		{
			candidates.push_back(t);
			totalWeight += t->m_weight;
		}

		bool placed = false;

		while(!candidates.empty() && !placed)
		{
			// Pick weighted random candidate
			const RoomConnection* connection = NULL;

			if(candidates.size() > 1)
			{
				tpublic::UniformDistribution<uint32_t> distribution(1, totalWeight);
				uint32_t roll = distribution(m_random);
				uint32_t sum = 0;

				for(size_t i = 0; i < candidates.size(); i++)
				{
					const RoomConnection* candidate = candidates[i];
					sum += candidate->m_weight;
					if(roll <= sum)
					{
						connection = candidate;
						assert(totalWeight >= connection->m_weight);
						totalWeight -= connection->m_weight;
						candidates.erase(candidates.begin() + i);
						break;
					}
				}
			}
			else
			{
				connection = candidates[0];
				candidates.clear();
			}

			//printf("trying connection %s\n", connection->m_debugName.c_str());

			// Try generating this room
			const Room* room = m_sourceRooms[connection->m_roomIndex];
			std::unique_ptr<GeneratedRoom> generatedRoom(_GenerateRoom(room));

			//printf("generated room: %s (%ux%u)\n", 
			//	generatedRoom->m_mapSegmentInstance.m_mapSegment->m_name.c_str(),
			//	generatedRoom->m_mapSegmentInstance.m_size.m_x, generatedRoom->m_mapSegmentInstance.m_size.m_y);

			// Find matching connectors
			struct Match
			{
				const MapSegmentInstance::Connector*	m_connector = NULL;
				Vec2									m_offset;
			};
			std::vector<Match> matchingConnectors;

			const Data::MapSegmentConnector* fromConnector = m_manifest->GetById<Data::MapSegmentConnector>(connection->m_mapSegmentConnectorId);

			for(const std::unique_ptr<MapSegmentInstance::Connector>& connector : generatedRoom->m_mapSegmentInstance.m_connectors)
			{				
				if(!fromConnector->ConnectsWith(connector->m_mapSegmentConnectorId))
					continue;

				if(connector->m_positions.size() == generatedRoomConnector->m_connector->m_positions.size())
				{
					Vec2 firstPosition = openConnector.m_generatedRoom->m_min + generatedRoomConnector->m_connector->m_positions[0];
					bool match = true;

					for(size_t i = 1; i < connector->m_positions.size() && match; i++)
					{
						if(connector->m_positions[i] != (connector->m_positions[i] - firstPosition))
							match = false;
					}

					if(match)
					{
						Vec2 offset = firstPosition - connector->m_positions[0];

						matchingConnectors.push_back({ connector.get(), offset });
					}
				}
			}

			while(matchingConnectors.size() > 0 && !placed)
			{
				Match match;

				if(matchingConnectors.size() > 1)
				{
					tpublic::UniformDistribution<size_t> distribution(0, matchingConnectors.size() - 1);
					size_t i = distribution(m_random);
					match = matchingConnectors[i];
					matchingConnectors.erase(matchingConnectors.begin() + i);
				}			
				else
				{
					match = matchingConnectors[0];
					matchingConnectors.clear();
				}

				//printf("offset %d %d\n", match.m_offset.m_x, match.m_offset.m_y);

				generatedRoom->m_min = match.m_offset;
				generatedRoom->m_max = generatedRoom->m_min + generatedRoom->m_mapSegmentInstance.m_size;

				if(!_CheckOverlap(generatedRoom.get()))
				{
					//printf("ok\n");

					for (size_t i = 0; i < generatedRoom->m_connectors.size(); i++)
					{
						if(generatedRoom->m_connectors[i]->m_connector != match.m_connector)
						{
							OpenConnector t;
							t.m_connectorIndex = i;
							t.m_generatedRoom = generatedRoom.get();
							m_openConnectors.push_back(t);
						}
					}

					_AddGeneratedRoom(generatedRoom);

					placed = true;
				}
				else
				{
					//printf("failed\n");
				}
			}	
		}

		if(!placed && openConnector.m_attempts < 5)
		{
			openConnector.m_attempts++;
			m_openConnectors.push_back(openConnector);
		}
	}

	void		
	Dungeon::Builder::DebugImageOutput(
		const char*						aPath) const
	{
		Image image((uint32_t)(m_max.m_x - m_min.m_x), (uint32_t)(m_max.m_y - m_min.m_y));
		image.Clear({ 0, 0, 0, 255 });

		for(const std::unique_ptr<GeneratedRoom>& generatedRoom : m_generatedRooms)
		{
			Image roomImage;
			generatedRoom->m_mapSegmentInstance.CreateDebugImage(m_manifest, roomImage);

			Vec2 imagePosition = generatedRoom->m_min - m_min;

			image.InsertBlended((uint32_t)imagePosition.m_x, (uint32_t)imagePosition.m_y, roomImage);
		}

		image.Grow(34, Image::RGBA{ 0, 32, 0, 255 }, m_min);
		image.SavePNG(aPath);
	}

	MapData* 
	Dungeon::Builder::CreateMapData()
	{
		std::unique_ptr<MapData> mapData = std::make_unique<MapData>();
		mapData->CopyFrom(m_sourceMapData);
		assert(mapData->m_tileMap == NULL);

		mapData->m_width = m_max.m_x - m_min.m_x;
		mapData->m_height = m_max.m_y - m_min.m_y;
		assert(mapData->m_width > 0 && mapData->m_height > 0);

		mapData->m_tileMap = new uint32_t[mapData->m_width * mapData->m_height];
		memset(mapData->m_tileMap, 0, mapData->m_width * mapData->m_height * sizeof(uint32_t));

		for (const std::unique_ptr<GeneratedRoom>& generatedRoom : m_generatedRooms)
		{
			Vec2 dstPosition = generatedRoom->m_min - m_min;

			for (int32_t y = 0; y < generatedRoom->m_mapSegmentInstance.m_size.m_y; y++)
			{
				const uint32_t* src = &generatedRoom->m_mapSegmentInstance.m_tileMap[y * generatedRoom->m_mapSegmentInstance.m_size.m_x];

				assert(dstPosition.m_x + generatedRoom->m_mapSegmentInstance.m_size.m_x <= mapData->m_width);
				assert(dstPosition.m_y < mapData->m_height);

				uint32_t* dst = &mapData->m_tileMap[dstPosition.m_x + dstPosition.m_y * mapData->m_width];
				memcpy(dst, src, sizeof(uint32_t) * generatedRoom->m_mapSegmentInstance.m_size.m_x);

				dstPosition.m_y++;
			}

			for(const Data::MapSegment::Object& object : generatedRoom->m_mapSegmentInstance.m_objects)
			{
				switch(object.m_type)
				{
				case Data::MapSegment::OBJECT_TYPE_PORTAL:
					{
						MapData::Portal t;
						t.m_id = object.m_id;
						t.m_x = generatedRoom->m_min.m_x + object.m_position.m_x;
						t.m_y = generatedRoom->m_min.m_y + object.m_position.m_y;
						mapData->m_portals.push_back(t);
					}
					break;

				case Data::MapSegment::OBJECT_TYPE_PLAYER_SPAWN:
					{
						MapData::PlayerSpawn t;
						t.m_id = object.m_id;
						t.m_x = generatedRoom->m_min.m_x + object.m_position.m_x;
						t.m_y = generatedRoom->m_min.m_y + object.m_position.m_y;
						mapData->m_playerSpawns.push_back(t);
					}
					break;

				case Data::MapSegment::OBJECT_TYPE_ENTITY_SPAWN:
					{
						MapData::EntitySpawn t;
						t.m_mapEntitySpawnId = object.m_id;
						t.m_x = generatedRoom->m_min.m_x + object.m_position.m_x;
						t.m_y = generatedRoom->m_min.m_y + object.m_position.m_y;
						mapData->m_entitySpawns.push_back(t);
					}
					break;

				default:
					assert(false);
					break;
				}
			}
		}

		{
			uint32_t* p = mapData->m_tileMap;

			for (int32_t y = 0; y < mapData->m_height; y++)
			{
				for (int32_t x = 0; x < mapData->m_width; x++)
				{
					if (*p == 0)
						*p = mapData->m_mapInfo.m_defaultTileSpriteId;

					p++;
				}
			}
		}

		return mapData.release();
	}

	void			
	Dungeon::Builder::_AddGeneratedRoom(
		std::unique_ptr<GeneratedRoom>&	aGeneratedRoom)
	{
		if (aGeneratedRoom->m_min.m_x < m_min.m_x)
			m_min.m_x = aGeneratedRoom->m_min.m_x;
		if (aGeneratedRoom->m_min.m_y < m_min.m_y)
			m_min.m_y = aGeneratedRoom->m_min.m_y;
		if (aGeneratedRoom->m_max.m_x > m_max.m_x)
			m_max.m_x = aGeneratedRoom->m_max.m_x;
		if (aGeneratedRoom->m_max.m_y > m_max.m_y)
			m_max.m_y = aGeneratedRoom->m_max.m_y;

		m_generatedRooms.push_back(std::move(aGeneratedRoom));
	}

	Dungeon::Builder::GeneratedRoom* 
	Dungeon::Builder::_GenerateRoom(
		const Room*						aSourceRoom)
	{
		TP_CHECK(aSourceRoom->m_totalSegmentWeight > 0, "No possible map segments for room.");

		std::unique_ptr<GeneratedRoom> generatedRoom = std::make_unique<GeneratedRoom>();

		// Pick weighted random map segment for room
		const Data::MapSegment* mapSegment = NULL;

		{
			tpublic::UniformDistribution<uint32_t> distribution(1, aSourceRoom->m_totalSegmentWeight);
			uint32_t roll = distribution(m_random);			
			uint32_t weightSum = 0;

			for(const RoomSegment& roomSegment : aSourceRoom->m_segments)
			{
				assert(roomSegment.m_weight > 0);
				weightSum += roomSegment.m_weight;

				if(roll <= weightSum)
				{
					mapSegment = m_manifest->GetById<Data::MapSegment>(roomSegment.m_mapSegmentId);
					break;
				}
			}

			assert(mapSegment != NULL);
		}

		// Generate map segment instance
		generatedRoom->m_mapSegmentInstance.Generate(m_manifest, mapSegment, m_random);

		generatedRoom->m_min = { 0, 0 };
		generatedRoom->m_max = generatedRoom->m_mapSegmentInstance.m_size;

		for(const std::unique_ptr<MapSegmentInstance::Connector>& connector : generatedRoom->m_mapSegmentInstance.m_connectors)
		{
			std::unique_ptr<GeneratedRoomConnector> generatedRoomConnector = std::make_unique<GeneratedRoomConnector>();
			generatedRoomConnector->m_mapSegmentConnectorId = connector->m_mapSegmentConnectorId;
			generatedRoomConnector->m_connector = connector.get();

			for (const RoomConnection& roomConnection : aSourceRoom->m_connections)
			{
				if(roomConnection.m_mapSegmentConnectorId == connector->m_mapSegmentConnectorId)
					generatedRoomConnector->m_possibleConnections.push_back(&roomConnection);
			}

			if(generatedRoomConnector->m_possibleConnections.size() > 0)
				generatedRoom->m_connectors.push_back(std::move(generatedRoomConnector));
		}

		return generatedRoom.release();
	}

	bool			
	Dungeon::Builder::_CheckOverlap(
		const GeneratedRoom* aGeneratedRoom) const
	{
		const GeneratedRoom* roomA = aGeneratedRoom;

		for(const std::unique_ptr<GeneratedRoom>& existing : m_generatedRooms)
		{		
			const GeneratedRoom* roomB = existing.get();

			Vec2 combinedMin = { Base::Min(roomA->m_min.m_x, roomB->m_min.m_x), Base::Min(roomA->m_min.m_y, roomB->m_min.m_y) };
			Vec2 combinedMax = { Base::Max(roomA->m_max.m_x, roomB->m_max.m_x), Base::Max(roomA->m_max.m_y, roomB->m_max.m_y) };

			if(combinedMax.m_x - combinedMin.m_x >= (roomB->m_max.m_x - roomB->m_min.m_x) + (roomA->m_max.m_x - roomA->m_min.m_x))
				continue;

			if (combinedMax.m_y - combinedMin.m_y >= (roomB->m_max.m_y - roomB->m_min.m_y) + (roomA->m_max.m_y - roomA->m_min.m_y))
				continue;

			// Bounding boxes overlap, we'll have to check contents
			Vec2 overlapMin = combinedMin;
			Vec2 overlapMax = combinedMax;

			if(roomA->m_min.m_x != combinedMin.m_x)
				overlapMin.m_x = roomA->m_min.m_x;
			else if (roomB->m_min.m_x != combinedMin.m_x)
				overlapMin.m_x = roomB->m_min.m_x;

			if (roomA->m_min.m_y != combinedMin.m_y)
				overlapMin.m_y = roomA->m_min.m_y;
			else if (roomB->m_min.m_y != combinedMin.m_y)
				overlapMin.m_y = roomB->m_min.m_y;

			if (roomA->m_max.m_x != combinedMax.m_x)
				overlapMax.m_x = roomA->m_max.m_x;
			else if (roomB->m_max.m_x != combinedMax.m_x)
				overlapMax.m_x = roomB->m_max.m_x;

			if (roomA->m_max.m_y != combinedMax.m_y)
				overlapMax.m_y = roomA->m_max.m_y;
			else if (roomB->m_max.m_y != combinedMax.m_y)
				overlapMax.m_y = roomB->m_max.m_y;

			for(int32_t y = overlapMin.m_y; y < overlapMax.m_y; y++)
			{
				int32_t yA = y - roomA->m_min.m_y;
				int32_t yB = y - roomB->m_min.m_y;

				for (int32_t x = overlapMin.m_x; x < overlapMax.m_x; x++)
				{
					int32_t xA = x - roomA->m_min.m_x;
					int32_t xB = x - roomB->m_min.m_x;

					uint32_t spriteAId = roomA->m_mapSegmentInstance.m_tileMap[xA + yA * roomA->m_mapSegmentInstance.m_size.m_x];
					uint32_t spriteBId = roomB->m_mapSegmentInstance.m_tileMap[xB + yB * roomB->m_mapSegmentInstance.m_size.m_x];

					if(spriteAId != 0 && spriteBId != 0 && spriteAId != spriteBId)
						return true;
				}
			}
		}

		return false;
	}

}
