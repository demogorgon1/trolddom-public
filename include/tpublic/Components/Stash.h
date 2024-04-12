#pragma once

#include "../ComponentBase.h"
#include "../ItemList.h"

namespace tpublic
{

	namespace Components
	{

		struct Stash
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_STASH;
			static const uint8_t FLAGS = FLAG_REPLICATE_ONLY_ON_REQUEST | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			static const uint32_t SOFT_SIZE_LIMIT = 16;		

			enum Field : uint32_t
			{
				FIELD_ITEMS
			};

			static void
			CreateSchema(
				ComponentSchema*					aSchema)
			{
				aSchema->DefineCustomObjectNoSource<ItemList>(FIELD_ITEMS, offsetof(Stash, m_items));
			}

			// Public data
			ItemList		m_items;
		};
	}

}