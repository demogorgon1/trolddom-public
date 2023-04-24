#pragma once

#include "../Component.h"
#include "../ThreatTable.h"

namespace kpublic
{

	namespace Components
	{

		struct ThreatTarget
			: public ComponentBase
		{			
			static const Component::Id ID = Component::ID_THREAT_TARGET;
			static const uint8_t FLAGS = 0;

			static const uint32_t PING_INTERVAL_TICKS = 10;

			ThreatTarget()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual
			~ThreatTarget()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* /*aStream*/) const override
			{
			}

			bool
			FromStream(
				IReader* /*aStream*/) override
			{
				return true;
			}

			// Public data
			ThreatTable		m_table;
			uint32_t		m_lastPingTick = 0;
		};
	}

}