#pragma once

#include "../Component.h"
#include "../Cooldowns.h"

namespace tpublic
{

	namespace Components
	{

		struct PlayerCooldowns
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_COOLDOWNS;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_VOLATILE;

			enum Field
			{
				FIELD_COOLDOWNS
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectNoSource<Cooldowns>(FIELD_COOLDOWNS, offsetof(PlayerCooldowns, m_cooldowns));
			}
			
			// Public data
			Cooldowns		m_cooldowns;
		};
	}

}