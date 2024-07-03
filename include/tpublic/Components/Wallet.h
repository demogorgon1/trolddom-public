#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Wallet
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_WALLET;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			enum Field
			{
				FIELD_CASH
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_INT64, FIELD_CASH, NULL, offsetof(Wallet, m_cash));
			}

			void
			Reset()
			{
				m_cash = 0;

				m_loot = 0;
			}

			// Public data
			int64_t			m_cash = 0;

			// Internal server
			int64_t			m_loot = 0;
		};
	}

}