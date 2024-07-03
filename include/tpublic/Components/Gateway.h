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

			enum Field
			{
				FIELD_MAPS,
				FIELD_ACTIVE_GATEWAY_INSTANCE,
				FIELD_SCHEDULE,
				FIELD_USE_POSITION_FOR_RANDOMIZATION,
				FIELD_PLAYER_WORLDS
			};

			struct Schedule
			{
				void
				FromSource(
					const SourceNode*	aSource)
				{
					m_timeSeedType = TimeSeed::StringToType(aSource->GetIdentifier());
					TP_VERIFY(m_timeSeedType != TimeSeed::INVALID_TYPE, aSource->m_debugInfo, "'%s' is not a valid time seed type.", aSource->GetIdentifier());
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_timeSeedType);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadPOD(m_timeSeedType))
						return false;
					return true;
				}

				// Public data
				TimeSeed::Type		m_timeSeedType = TimeSeed::TYPE_DAILY;
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32_ARRAY, FIELD_MAPS, "maps", offsetof(Gateway, m_maps))->SetDataType(DataType::ID_MAP);
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
			std::vector<uint32_t>	m_maps;
			GatewayInstance			m_activeGatewayInstance;
			Schedule				m_schedule;
			bool					m_usePositionForRandomization = false;
			bool					m_playerWorlds = false;
		};
	}

}