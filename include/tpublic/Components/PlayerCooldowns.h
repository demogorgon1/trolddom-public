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
			
			PlayerCooldowns()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~PlayerCooldowns()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* aStream) const override
			{
				m_cooldowns.ToStream(aStream);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if(!m_cooldowns.FromStream(aStream))
					return false;
				return true;
			}

			// Public data
			Cooldowns		m_cooldowns;
		};
	}

}