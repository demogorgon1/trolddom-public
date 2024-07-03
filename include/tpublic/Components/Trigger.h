#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Trigger
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_TRIGGER;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_NONE;

			enum Field
			{
				FIELD_LOCK_OUT_TICKS,
				FIELD_TRIGGER_MANUAL_OBJECTIVE_ID,
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_LOCK_OUT_TICKS, "lock_out_ticks", offsetof(Trigger, m_lockOutTicks));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_TRIGGER_MANUAL_OBJECTIVE_ID, "trigger_manual_objective", offsetof(Trigger, m_triggerManualObjectiveId))->SetDataType(DataType::ID_OBJECTIVE);
			}

			void
			Reset()
			{
				m_lockOutTicks = 0;
				m_triggerManualObjectiveId = 0;

				m_lockOutTable.clear();
			}

			// Public data
			int32_t			m_lockOutTicks = 0;
			uint32_t		m_triggerManualObjectiveId = 0;

			// Internal
			typedef std::unordered_map<uint32_t, int32_t> LockOutTable;
			LockOutTable	m_lockOutTable;
		};
	}

}