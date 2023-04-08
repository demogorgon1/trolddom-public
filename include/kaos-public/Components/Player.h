#pragma once

#include "Component.h"
#include "Vec2.h"

namespace kaos_public
{

	namespace Components
	{

		struct Player
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER;
			static const uint8_t FLAGS = FLAG_SHARED_OWNER | FLAG_SHARED_OTHERS | FLAG_PLAYER_ONLY;

			Player()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual
			~Player()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* aStream) const override
			{
				aStream->WriteUInt(m_classId);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if(!aStream->ReadUInt(m_classId))
					return false;
				return true;
			}

			// Public data
			uint32_t		m_classId = 0;
		};
	}

}