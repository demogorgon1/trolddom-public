#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
{

	class Openable
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_OPENABLE;
		static const int32_t UPDATE_INTERVAL = 10;

						Openable(
							const Manifest*		aManifest);
		virtual			~Openable();

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