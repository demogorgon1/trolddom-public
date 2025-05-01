#pragma once

#include "../Component.h"
#include "../ComponentBase.h"
#include "../Cooldowns.h"

namespace tpublic
{

	namespace Components
	{

		struct PlayerCooldowns
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_COOLDOWNS;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_VOLATILE;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

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

			void
			Reset()
			{
				m_cooldowns.m_entries.clear();
			}
			
			// Public data
			Cooldowns		m_cooldowns;
		};
	}

}