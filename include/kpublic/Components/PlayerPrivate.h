#pragma once

#include "../Component.h"

namespace kpublic
{

	namespace Components
	{

		struct PlayerPrivate
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_PRIVATE;
			static const uint8_t FLAGS = FLAG_PRIVATE | FLAG_PLAYER_ONLY | FLAG_PERSISTENT;
			
			PlayerPrivate()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual
			~PlayerPrivate()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* aStream) const override
			{
				aStream->WriteUInt(m_level);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if(!aStream->ReadUInt(m_level))
					return false;
				return true;
			}

			// Public data
			uint32_t		m_level = 1;
		};
	}

}