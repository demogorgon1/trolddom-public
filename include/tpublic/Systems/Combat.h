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
							const SystemData*	aData);
		virtual			~Combat();

		// SystemBase implementation
		EntityState::Id	UpdatePrivate(
							uint32_t			aEntityId,
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							int32_t				aTicksInState,
							ComponentBase**		aComponents,
							Context*			aContext) override;
		void			UpdatePublic(
							uint32_t			aEntityId,
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							int32_t				aTicksInState,
							ComponentBase**		aComponents,
							Context*			aContext) override;

	};

}