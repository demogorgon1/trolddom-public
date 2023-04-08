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

			Player()
				: ComponentBase(ID)
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