#pragma once

#include "../SystemBase.h"

namespace kaos_public::Systems
{

	class Combat
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_COMBAT;

						Combat(
							const Manifest*		aManifest);
		virtual			~Combat();

		// SystemBase implementation
		EntityState::Id	Update(
							EntityState::Id		aEntityState,
							ComponentBase**		aComponents,
							Context*			aContext) override;

	};

}