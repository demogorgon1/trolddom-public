#pragma once

#include "../Component.h"

namespace tpublic
{

	namespace Components
	{

		struct DisplayName
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_DISPLAY_NAME;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OTHERS | FLAG_REPLICATE_TO_OWNER;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			enum Field
			{
				FIELD_STRING
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_STRING, FIELD_STRING, "string", offsetof(DisplayName, m_string));
			}

			DisplayName()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual 
			~DisplayName()
			{

			}

			// Public data
			std::string		m_string;
		};
	}

}