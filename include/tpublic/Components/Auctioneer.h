#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Auctioneer
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_AUCTIONEER;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			static void
			CreateSchema(
				ComponentSchema* /*aSchema*/)
			{
			}

			void
			Reset()
			{

			}

			// Public data
		};
	}

}