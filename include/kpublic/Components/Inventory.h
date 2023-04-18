#pragma once

#include "Component.h"

namespace kpublic
{

	namespace Components
	{

		struct Inventory
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_INVENTORY;
			static const uint8_t FLAGS = FLAG_PRIVATE | FLAG_PLAYER_ONLY;

			Inventory()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual
			~Inventory()
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