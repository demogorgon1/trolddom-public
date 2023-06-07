#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
{

	class Environment
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_ENVIRONMENT;

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
							ComponentBase**		aComponents,
							Context*			aContext) override;

	};

}