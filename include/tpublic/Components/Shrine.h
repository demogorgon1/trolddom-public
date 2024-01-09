#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Shrine
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_SHRINE;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_NONE;

			enum Field
			{
				FIELD_PANTHEON_ID
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_PANTHEON_ID, "pantheon", offsetof(Shrine, m_pantheonId))->SetDataType(DataType::ID_PANTHEON);
			}

			// Public data
			uint32_t	m_pantheonId = 0;

			// Internal
			uint32_t	m_deityId = 0;
		};
	}

}