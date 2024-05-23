#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
{

	class Trigger
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_TRIGGER;
		static const int32_t UPDATE_INTERVAL = 100;

						Trigger(
							const SystemData*	aData);
		virtual			~Trigger();

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