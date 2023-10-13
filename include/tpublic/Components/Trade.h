#pragma once

#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Trade
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_TRADE;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			enum Field
			{
				FIELD_OTHER_CHARACTER_ID
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_OTHER_CHARACTER_ID, NULL, offsetof(Trade, m_otherCharacterId));
			}

			// Public data
			uint32_t		m_otherCharacterId = 0;
		};
	}

}