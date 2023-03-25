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
			{

			}

			virtual 
			~Position()
			{

			}

			// Public data
			Vec2	m_position;
		};
	}

}