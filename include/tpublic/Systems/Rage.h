#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
{

	class Rage
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_RAGE;

						Rage(
							const SystemData*	aData);
		virtual			~Rage();

		// SystemBase implementation
		EntityState::Id	UpdatePrivate(
							uint32_t			aEntityId,
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							int32_t				aTicksInState,
							ComponentBase**		aComponents,
							Context*			aContext) override;

	private:
	};

}