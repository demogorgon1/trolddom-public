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
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS | FLAG_PUBLIC;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

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