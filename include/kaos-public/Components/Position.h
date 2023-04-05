#pragma once

#include "Component.h"
#include "Vec2.h"

namespace kaos_public
{

	namespace Components
	{

		struct Position
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_POSITION;

			Position()
				: ComponentBase(ID)
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