#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
{

	class Environment
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_ENVIRONMENT;

		static const int32_t SPAWN_TICKS = 3;
		static const int32_t DESPAWN_TICKS = 3;

						Environment(
							const Manifest*		aManifest);
		virtual			~Environment();

		// SystemBase implementation
		void			Init(
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							ComponentBase**		aComponents,
							int32_t				aTick) override;
		EntityState::Id	UpdatePrivate(
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							int32_t				aTicksInState,
							ComponentBase**		aComponents,
							Context*			aContext) override;

	};

}