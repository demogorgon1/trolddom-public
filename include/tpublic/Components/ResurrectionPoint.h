#pragma once

#include "../Component.h"

namespace tpublic
{

	namespace Components
	{

		struct ResurrectionPoint
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_RESURRECTION_POINT;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_NONE;

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