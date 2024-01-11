#pragma once

#include "../Component.h"
#include "../ComponentBase.h"
#include "../RangeBasedSet.h"

namespace tpublic
{

	namespace Components
	{

		struct DeityDiscovery
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_DEITY_DISCOVERY;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			enum Field
			{
				FIELD_DEITIES
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectNoSource<RangeBasedSet<uint32_t>>(FIELD_DEITIES, offsetof(DeityDiscovery, m_deities));
			}

			// Public data
			RangeBasedSet<uint32_t>		m_deities;
		};
	}

}