#pragma once

#include "Component.h"

namespace kpublic
{

	namespace Components
	{

		struct EquippedItems
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_EQUIPPED_ITEMS;
			static const uint8_t FLAGS = FLAG_PRIVATE | FLAG_PUBLIC | FLAG_PLAYER_ONLY | FLAG_PERSISTENT;

			EquippedItems()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual
			~EquippedItems()
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