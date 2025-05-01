#pragma once

#include "../Component.h"
#include "../ComponentBase.h"
#include "../GatewayInstance.h"
#include "../TimeSeed.h"

namespace tpublic
{

	namespace Components
	{

		struct Gateway
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_GATEWAY;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			struct Schedule
			{
				void
				FromSource(
					const SourceNode*	aSource)
				{
					if(aSource->IsIdentifier("none"))
					{
						m_noSchedule = true;
					}
					else
					{
						m_timeSeedType = TimeSeed::StringToType(aSource->GetIdentifier());
						TP_VERIFY(m_timeSeedType != TimeSeed::INVALID_TYPE, aSource->m_debugInfo, "'%s' is not a valid time seed type.", aSource->GetIdentifier());
					}
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_timeSeedType);
					aWriter->WritePOD(m_noSchedule);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadPOD(m_timeSeedType))
						return false;
					if(!aReader->ReadBool(m_noSchedule))
						return false;
					return true;
				}

				// Public data
				bool				m_noSchedule = false;
				TimeSeed::Type		m_timeSeedType = TimeSeed::TYPE_DAILY;
			};

			struct Map
			{
				void
				FromSource(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "map")
							m_mapId = aChild->GetId(DataType::ID_MAP);
						else if (aChild->m_name == "map_player_spawn")
							m_mapPlayerSpawnId = aChild->GetId(DataType::ID_MAP_PLAYER_SPAWN);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
					
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_mapId);
					aWriter->WriteUInt(m_mapPlayerSpawnId);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if (!aReader->ReadUInt(m_mapId))
						return false;
					if (!aReader->ReadUInt(m_mapPlayerSpawnId))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_mapId = 0;
				uint32_t			m_mapPlayerSpawnId = 0;
			};

			enum Field
			{
				FIELD_MAPS,
				FIELD_ACTIVE_GATEWAY_INSTANCE,
				FIELD_SCHEDULE,
				FIELD_USE_POSITION_FOR_RANDOMIZATION,
				FIELD_PLAYER_WORLDS
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->DefineCustomObjects<Map>(FIELD_MAPS, "maps", offsetof(Gateway, m_maps))->SetDataType(DataType::ID_MAP);
				aSchema->DefineCustomObjectNoSource<GatewayInstance>(FIELD_ACTIVE_GATEWAY_INSTANCE, offsetof(Gateway, m_activeGatewayInstance));
				aSchema->DefineCustomObject<Schedule>(FIELD_SCHEDULE, "schedule", offsetof(Gateway, m_schedule));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_USE_POSITION_FOR_RANDOMIZATION, "use_position_for_randomization", offsetof(Gateway, m_usePositionForRandomization));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_PLAYER_WORLDS, "player_worlds", offsetof(Gateway, m_playerWorlds));
			}

			void
			Reset()
			{
				m_maps.clear();
				m_activeGatewayInstance = GatewayInstance();
				m_schedule = Schedule();
				m_usePositionForRandomization = false;
				m_playerWorlds = false;
			}

			// Public data
			std::vector<Map>		m_maps;
			GatewayInstance			m_activeGatewayInstance;
			Schedule				m_schedule;
			bool					m_usePositionForRandomization = false;
			bool					m_playerWorlds = false;
		};
	}

}