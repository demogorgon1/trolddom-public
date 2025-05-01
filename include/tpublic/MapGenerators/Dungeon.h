#pragma once

#include "../MapGeneratorBase.h"
#include "../MapSegmentInstance.h"

namespace tpublic::MapGenerators
{

	class Dungeon
		: public MapGeneratorBase
	{
	public:
		static const MapGenerator::Id ID = MapGenerator::ID_DUNGEON;

		enum RoomFlag : uint8_t 
		{
			ROOM_FLAG_ROOT		= 0x01
		};

		static uint8_t
		StringToRoomFlag(
			const char*								aString)
		{
			if(strcmp(aString, "root") == 0)
				return ROOM_FLAG_ROOT;
			return 0;
		}		

		struct RoomSegment
		{
			RoomSegment()
			{

			}

			RoomSegment(
				const SourceNode*					aSource)
			{
				if(aSource->m_annotation)
					m_weight = aSource->m_annotation->GetUInt32();

				m_mapSegmentId = aSource->GetId(DataType::ID_MAP_SEGMENT);
			}

			void
			ToStream(
				IWriter*							aWriter) const
			{
				aWriter->WriteUInt(m_weight);
				aWriter->WriteUInt(m_mapSegmentId);
			}

			bool
			FromStream(
				IReader*							aReader)
			{
				if (!aReader->ReadUInt(m_weight))
					return false;
				if (!aReader->ReadUInt(m_mapSegmentId))
					return false;
				return true;
			}

			// Public data
			uint32_t						m_weight = 1;
			uint32_t						m_mapSegmentId = 0;
		};

		struct RoomConnection
		{
			RoomConnection()
			{

			}

			RoomConnection(
				const SourceNode*					aSource)
			{
				TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "No map segment connector annotation specified.");
				m_mapSegmentConnectorId = aSource->m_annotation->GetId(DataType::ID_MAP_SEGMENT_CONNECTOR);
				m_roomName = aSource->m_name;

				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "weight")
						m_weight = aChild->GetUInt32();
					else if (aChild->m_name == "debug_name")
						m_debugName = aChild->GetString();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*							aWriter) const
			{
				aWriter->WriteUInt(m_mapSegmentConnectorId);
				aWriter->WriteUInt(m_roomIndex);
				aWriter->WriteUInt(m_weight);
			}

			bool
			FromStream(
				IReader*							aReader)
			{
				if (!aReader->ReadUInt(m_mapSegmentConnectorId))
					return false;
				if (!aReader->ReadUInt(m_roomIndex))
					return false;
				if (!aReader->ReadUInt(m_weight))
					return false;
				return true;
			}

			// Public data
			uint32_t						m_mapSegmentConnectorId = 0;
			uint32_t						m_roomIndex = UINT32_MAX;
			uint32_t						m_weight = 1;
			std::string						m_debugName;

			// Only for building, converted to index
			std::string						m_roomName;
		};

		struct Room
		{						
			Room()
			{

			}

			Room(
				const SourceNode*					aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "flags")
					{
						aChild->GetArray()->ForEachChild([&](
							const SourceNode* aFlag)
						{
							uint8_t flag = StringToRoomFlag(aFlag->GetIdentifier());
							TP_VERIFY(flag != 0, aChild->m_debugInfo, "'%s' is not a valid flag.", aFlag->GetIdentifier());
							m_flags |= flag;
						});						
					}
					else if(aChild->m_name == "segment")
					{
						m_segments.push_back(RoomSegment(aChild));
					}
					else if (aChild->m_tag == "connection")
					{
						m_connections.push_back(RoomConnection(aChild));
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});

				for(const RoomSegment& segment : m_segments)
					m_totalSegmentWeight += segment.m_weight;

				for (const RoomConnection& connection : m_connections)
					m_totalConnectionWeight += connection.m_weight;
			}

			void
			ToStream(
				IWriter*							aWriter) const
			{
				aWriter->WritePOD(m_flags);
				aWriter->WriteUInt(m_totalSegmentWeight);
				aWriter->WriteUInt(m_totalConnectionWeight);
				aWriter->WriteObjects(m_segments);
				aWriter->WriteObjects(m_connections);
			}

			bool
			FromStream(
				IReader*							aReader)
			{
				if(!aReader->ReadPOD(m_flags))
					return false;
				if (!aReader->ReadUInt(m_totalSegmentWeight))
					return false;
				if (!aReader->ReadUInt(m_totalConnectionWeight))
					return false;
				if (!aReader->ReadObjects(m_segments))
					return false;
				if (!aReader->ReadObjects(m_connections))
					return false;
				return true;
			}

			const RoomConnection*
			GetRoomConnection(
				uint32_t							aMapSegmentConnectorId) const
			{
				for(const RoomConnection& t : m_connections)
				{
					if(t.m_mapSegmentConnectorId == aMapSegmentConnectorId)
						return &t;
				}
				return NULL;
			}

			// Public data
			uint8_t							m_flags = 0;
			uint32_t						m_totalSegmentWeight = 0;
			uint32_t						m_totalConnectionWeight = 0;
			std::vector<RoomSegment>		m_segments;
			std::vector<RoomConnection>		m_connections;
		};

					Dungeon();
		virtual		~Dungeon();
		
		// MapGeneratorBase implementation
		void		FromSource(
						const SourceNode*			aSource) override;
		void		ToStream(
						IWriter*					aWriter) const override;
		bool		FromStream(
						IReader*					aReader) override;
		bool		Build(
						const Manifest*				aManifest,
						MapGeneratorRuntime*		aMapGeneratorRuntime,
						uint32_t					aSeed,
						const MapData*				aSourceMapData,
						const char*					aDebugImagePath,
						std::unique_ptr<MapData>&	aOutMapData) const override;

	private:

		typedef std::unordered_map<std::string, uint32_t> RoomNameTable;
		RoomNameTable						m_roomNameTable;
		std::vector<std::unique_ptr<Room>>	m_rooms;

		class Builder
		{
		public:

			Builder(
				const Manifest*									aManifest,
				uint32_t										aSeed,
				const MapData*									aSourceMapData,
				const std::vector<std::unique_ptr<Room>>&		aSourceRooms)
				: m_random(aSeed)
				, m_manifest(aManifest)
				, m_sourceMapData(aSourceMapData)
			{
				for(const std::unique_ptr<Room>& room : aSourceRooms)
					m_sourceRooms.push_back(room.get());
			}

			void		GenerateRootRoom();
			void		GenerateAndAttachRoom();
			void		DebugImageOutput(
							const char*							aPath) const;
			MapData*	CreateMapData();

			// Data access
			bool		HasOpenConnectors() const { return !m_openConnectors.empty(); }

		private:

			struct GeneratedRoomConnector
			{
				uint32_t												m_mapSegmentConnectorId = 0;
				const MapSegmentInstance::Connector*					m_connector = NULL;
				std::vector<const RoomConnection*>						m_possibleConnections;
			};

			struct GeneratedRoom
			{
				GeneratedRoomConnector*
				GetOrCreateConnector(
					uint32_t									aMapSegmentConnectorId)
				{
					for(std::unique_ptr<GeneratedRoomConnector>& connector : m_connectors)
					{	
						if(connector->m_mapSegmentConnectorId == aMapSegmentConnectorId)
							return connector.get();
					}
					
					GeneratedRoomConnector* t = new GeneratedRoomConnector();
					t->m_mapSegmentConnectorId = aMapSegmentConnectorId;
					m_connectors.push_back(std::unique_ptr<GeneratedRoomConnector>(t));
					return t;
				}

				// Public data
				MapSegmentInstance										m_mapSegmentInstance;
				Vec2													m_min;
				Vec2													m_max;
				std::vector<std::unique_ptr<GeneratedRoomConnector>>	m_connectors;
			};

			struct OpenConnector
			{
				const GeneratedRoom*									m_generatedRoom = NULL;
				size_t													m_connectorIndex = 0;
				uint32_t												m_attempts = 0;
			};

			const Manifest*												m_manifest;
			std::mt19937												m_random;
			const MapData*												m_sourceMapData;
			std::vector<const Room*>									m_sourceRooms;

			std::vector<std::unique_ptr<GeneratedRoom>>					m_generatedRooms;
			Vec2														m_min;
			Vec2														m_max;

			std::vector<OpenConnector>									m_openConnectors;
			std::vector<OpenConnector>									m_failedConnectors;
			
			void			_AddGeneratedRoom(
								std::unique_ptr<GeneratedRoom>&	aGeneratedRoom);
			GeneratedRoom*	_GenerateRoom(
								const Room*						aSourceRoom);
			bool			_CheckOverlap(
								const GeneratedRoom*			aGeneratedRoom) const;
		};
	};


}