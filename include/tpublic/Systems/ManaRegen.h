#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
{

	class ManaRegen
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_MANA_REGEN;

						ManaRegen(
							const SystemData*	aData);
		virtual			~ManaRegen();

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