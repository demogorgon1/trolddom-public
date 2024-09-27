#pragma once

#include "../ComponentBase.h"
#include "../SourceEntityInstance.h"

namespace tpublic
{

	namespace Components
	{

		struct Owner
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_OWNER;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_NONE;

			static void
			CreateSchema(
				ComponentSchema*		/*aSchema*/)
			{
			}

			void
			Reset()
			{
				m_ownerSourceEntityInstance = SourceEntityInstance();
			}

			// Public data
			SourceEntityInstance		m_ownerSourceEntityInstance;
		};
	}

}