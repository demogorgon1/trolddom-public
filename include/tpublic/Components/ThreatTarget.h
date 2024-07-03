#pragma once

#include "../Component.h"
#include "../ThreatTable.h"

namespace tpublic
{

	namespace Components
	{

		struct ThreatTarget
			: public ComponentBase
		{			
			static const Component::Id ID = Component::ID_THREAT_TARGET;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_NONE;

			static const int32_t PING_INTERVAL_TICKS = 10;

			static void
			CreateSchema(
				ComponentSchema* /*aSchema*/)
			{
			}

			void
			Reset()
			{
				m_table.Clear();
				m_lastPingTick = 0;
			}

			// Public data
			// Not serialized, server only
			ThreatTable		m_table;
			int32_t			m_lastPingTick = 0;
		};
	}

}