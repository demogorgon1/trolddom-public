#include "../Pcheader.h"

#include <tpublic/MapGenerators/Dungeon.h>

#include <tpublic/Manifest.h>

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
		const SourceNode*			aSource) 
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
	}
	
	void		
	Dungeon::ToStream(
		IWriter*					aWriter) const
	{
		ToStreamBase(aWriter);

		aWriter->WriteObjectPointers(m_rooms);
	}
	
	bool		
	Dungeon::FromStream(
		IReader*					aReader) 
	{
		if(!FromStreamBase(aReader))
			return false;

		if(!aReader->ReadObjectPointers(m_rooms))
			return false;
		return true;
	}

	bool		
	Dungeon::Build(
		const Manifest*				aManifest,
		uint32_t					aSeed,
		const MapData*				aSourceMapData,
		std::unique_ptr<MapData>&	aOutMapData) const 
	{
		Builder builder(aManifest, aSeed, aSourceMapData, m_rooms);

		builder.GenerateRootRoom();

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
				std::uniform_int_distribution<size_t> distribution(0, possibleRootRooms.size() - 1);
				rootRoom = possibleRootRooms[distribution(m_random)];
			}
		}

		std::unique_ptr<GeneratedRoom> generatedRootRoom(_GenerateRoom(rootRoom));
	}

	MapData* 
	Dungeon::Builder::CreateMapData()
	{
		assert(false);
		return NULL;
	}

	Dungeon::Builder::GeneratedRoom* 
	Dungeon::Builder::_GenerateRoom(
		const Room*					aSourceRoom)
	{
		TP_CHECK(aSourceRoom->m_totalSegmentWeight > 0, "No possible map segments for room.");

		std::unique_ptr<GeneratedRoom> generatedRoom = std::make_unique<GeneratedRoom>();

		// Pick weighted random map segment for room
		const Data::MapSegment* mapSegment = NULL;

		{
			std::uniform_int_distribution<uint32_t> distribution(1, aSourceRoom->m_totalSegmentWeight);
			uint32_t roll = distribution(m_random);			
			uint32_t weightSum = 0;

			for(const RoomSegment& roomSegment : aSourceRoom->m_segments)
			{
				assert(roomSegment.m_weight > 0);
				weightSum += roomSegment.m_weight;

				if(roll <= weightSum)
				{
					mapSegment = m_manifest->m_mapSegments.GetById(roomSegment.m_mapSegmentId);
					break;
				}
			}

			assert(mapSegment != NULL);
		}

		// Generate map segment instance
		generatedRoom->m_mapSegmentInstance.Generate(mapSegment, m_random);

		return generatedRoom.release();
	}

}