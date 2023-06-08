#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
{

	class NPC
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_NPC;

		static const int32_t SPAWN_TICKS = 4;
		static const int32_t DESPAWN_TICKS = 4;

						NPC(
							const Manifest*		aManifest);
		virtual			~NPC();

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
		void			UpdatePublic(
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							int32_t				aTicksInState,
							ComponentBase**		aComponents,
							Context*			aContext) override;

	private:
	};

}