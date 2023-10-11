#pragma once

#include "../Component.h"
#include "../PlayerTag.h"

namespace tpublic
{

	namespace Components
	{

		struct Tag
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_TAG;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OTHERS;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			enum Field
			{
				FIELD_PLAYER_TAG
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectNoSource<PlayerTag>(FIELD_PLAYER_TAG, offsetof(Tag, m_playerTag));
			}

			// Public data
			PlayerTag		m_playerTag;
		};
	}

}