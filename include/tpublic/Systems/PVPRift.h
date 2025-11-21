#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
{

	class PVPRift
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_PVP_RIFT;
		static const int32_t UPDATE_INTERVAL = 20;

						PVPRift(
							const SystemData*	aData);
		virtual			~PVPRift();

		// SystemBase implementation
		EntityState::Id	UpdatePrivate(
							uint32_t			aEntityId,
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							int32_t				aTicksInState,
							ComponentBase**		aComponents,
							Context*			aContext) override;
	};

}