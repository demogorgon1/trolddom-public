#pragma once

#include "../Component.h"
#include "../Cooldowns.h"

namespace kpublic
{

	namespace Components
	{

		struct PlayerCooldowns
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_COOLDOWNS;
			static const uint8_t FLAGS = FLAG_PRIVATE | FLAG_PLAYER_ONLY | FLAG_PERSISTENT;
			
			PlayerCooldowns()
				: ComponentBase(ID, FLAGS)
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