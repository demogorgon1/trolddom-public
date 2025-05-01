#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct HoverText
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_HOVER_TEXT;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Field
			{
				FIELD_STRING
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_STRING, FIELD_STRING, "string", offsetof(HoverText, m_string));
			}

			void
			Reset()
			{
				m_string.clear();
			}

			// Public data
			std::string			m_string;
		};
	}

}