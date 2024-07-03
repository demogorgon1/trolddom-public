#pragma once

#include "../Component.h"
#include "../ComponentBase.h"
#include "../RangeBasedSet.h"

namespace tpublic
{

	namespace Components
	{

		struct ZoneDiscovery
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_ZONE_DISCOVERY;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			enum Field
			{
				FIELD_ZONES
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectNoSource<RangeBasedSet<uint32_t>>(FIELD_ZONES, offsetof(ZoneDiscovery, m_zones));
			}

			void
			Reset()
			{
				m_zones.Reset();
			}

			// Public data
			RangeBasedSet<uint32_t>		m_zones;
		};
	}

}