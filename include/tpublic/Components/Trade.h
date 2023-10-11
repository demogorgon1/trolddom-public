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
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

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

			Trade()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual 
			~Trade()
			{

			}

			// Public data
			uint32_t		m_otherCharacterId = 0;
		};
	}

}