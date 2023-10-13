#pragma once

#include "../ComponentBase.h"

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
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Field
			{
				FIELD_OWNER_ENTITY_INSTANCE_ID
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_OWNER_ENTITY_INSTANCE_ID, NULL, offsetof(Owner, m_ownerEntityInstanceId));
			}

			// Public data
			uint32_t		m_ownerEntityInstanceId = 0;
		};
	}

}