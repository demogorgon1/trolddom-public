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
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS | FLAG_PUBLIC;
			static const Persistence::Id PERSISTENCE = Persistence::ID_VOLATILE;

			enum Field : uint32_t
			{
				FIELD_POSITION,
				FIELD_BLOCK
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_VEC2, FIELD_POSITION, NULL, offsetof(Position, m_position));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_BLOCK, NULL, offsetof(Position, m_block));
			}

			Position()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual 
			~Position()
			{

			}

			// Public data
			Vec2	m_position;
			bool	m_block = false;
		};
	}

}