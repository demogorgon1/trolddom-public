#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Environment
			: public ComponentBase
		{			
			static const Component::Id ID = Component::ID_ENVIRONMENT;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_NONE;

			enum Field
			{
				FIELD_TICK_INTERVAL,
				FIELD_DURATION,
				FIELD_ABILITY
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_TICK_INTERVAL, "tick_interval", offsetof(Environment, m_tickInterval));
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_DURATION, "duration", offsetof(Environment, m_duration));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_ABILITY, "ability", offsetof(Environment, m_abilityId))->SetDataType(DataType::ID_ABILITY);
			}

			// Public data
			int32_t			m_tickInterval = 5;
			int32_t			m_duration = 10;
			uint32_t		m_abilityId = 0;			

			// Not serialized
			int32_t			m_lastUpdateTick = 0;
			int32_t			m_despawnTick = 0;
		};
	}

}