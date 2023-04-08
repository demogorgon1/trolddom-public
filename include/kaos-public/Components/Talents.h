#pragma once

#include "Component.h"

namespace kaos_public
{

	namespace Components
	{

		struct Talents
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_TALENTS;
			static const uint8_t FLAGS = FLAG_SHARED_OWNER | FLAG_SHARED_OTHERS | FLAG_PLAYER_ONLY | FLAG_PERSISTENT;

			Talents()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual
			~Talents()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* /*aStream*/) const override
			{
			}

			bool
			FromStream(
				IReader* /*aStream*/) override
			{
				return true;
			}

			// Public data
		};
	}

}