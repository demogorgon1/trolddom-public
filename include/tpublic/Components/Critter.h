#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Critter
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_CRITTER;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Field
			{
				FIELD_CRITTER_ID,
				FIELD_POSITION
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_CRITTER_ID, "critter", offsetof(Critter, m_critterId))->SetDataType(DataType::ID_CRITTER);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_POSITION, "position", offsetof(Critter, m_position));
			}

			void
			Reset()
			{
				m_critterId = 0;
				m_position = 0;
			}

			// Public data
			uint32_t			m_critterId = 0;
			uint32_t			m_position = 0;		
		};
	}

}