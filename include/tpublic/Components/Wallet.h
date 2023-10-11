#pragma once

#include "../Component.h"

namespace tpublic
{

	namespace Components
	{

		struct Wallet
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_WALLET;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;

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

			// Public data
			int64_t			m_cash = 0;
		};
	}

}