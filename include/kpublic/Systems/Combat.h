#pragma once

#include "../SystemBase.h"

namespace kpublic::Systems
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
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							ComponentBase**		aComponents,
							Context*			aContext) override;

	};

}