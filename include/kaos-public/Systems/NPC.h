#pragma once

#include "../SystemBase.h"

namespace kaos_public
{

	namespace Systems
	{

		class NPC
			: public SystemBase
		{
		public:
			static System::Id ID = System::ID_NPC;
		};

	}

}