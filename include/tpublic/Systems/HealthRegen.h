#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
{

	class HealthRegen
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_HEALTH_REGEN;
		static const int32_t UPDATE_INTERVAL = 20;

						HealthRegen(
							const Manifest*		aManifest);
		virtual			~HealthRegen();

		// SystemBase implementation
		EntityState::Id	UpdatePrivate(
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							int32_t				aTicksInState,
							ComponentBase**		aComponents,
							Context*			aContext) override;

	private:
	};

}