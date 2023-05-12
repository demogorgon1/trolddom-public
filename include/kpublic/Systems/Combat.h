#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
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
		EntityState::Id	UpdatePrivate(
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							ComponentBase**		aComponents,
							Context*			aContext) override;
		void			UpdatePublic(
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							ComponentBase**		aComponents,
							Context*			aContext) override;

	};

}