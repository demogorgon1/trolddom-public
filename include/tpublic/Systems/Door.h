#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
{

	class Door
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_DOOR;

						Door(
							const SystemData*	aData);
		virtual			~Door();

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