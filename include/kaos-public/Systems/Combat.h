#pragma once

#include "../SystemBase.h"

namespace kaos_public
{

	namespace Systems
	{

		class Combat
			: public SystemBase
		{
		public:
			static const System::Id ID = System::ID_COMBAT;
		};

	}

}