#pragma once

#include "../ComponentBase.h"
#include "../Vec2.h"

namespace kpublic
{

	namespace Components
	{

		struct Position
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_POSITION;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS | FLAG_PUBLIC;

			Position()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual 
			~Position()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* aStream) const override
			{
				m_position.ToStream(aStream);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if (!m_position.FromStream(aStream))
					return false;
				return true;
			}

			// Public data
			Vec2	m_position;
		};
	}

}