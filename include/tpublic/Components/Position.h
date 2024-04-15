#pragma once

#include "../ComponentBase.h"
#include "../Vec2.h"

namespace tpublic
{

	namespace Components
	{

		struct Position
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_POSITION;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_VOLATILE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Field : uint32_t
			{
				FIELD_POSITION,
				FIELD_BLOCK,
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_VEC2, FIELD_POSITION, NULL, offsetof(Position, m_position));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_BLOCK, NULL, offsetof(Position, m_block));
			}

			// Public data
			Vec2		m_position;
			bool		m_block = false;

			// Not serialized, internal server stuff
			uint32_t	m_lastMoveTick = 0;
			bool		m_detached = false;
			bool		m_updatedOnServer = false;
		};
	}

}