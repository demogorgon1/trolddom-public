#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
{

	class Shrine
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_SHRINE;
		static const int32_t UPDATE_INTERVAL = 10;

						Shrine(
							const SystemData*	aData);
		virtual			~Shrine();

		// SystemBase implementation
		void			Init(
							uint32_t			aEntityId,
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							ComponentBase**		aComponents,
							int32_t				aTick) override;
		EntityState::Id	UpdatePrivate(
							uint32_t			aEntityId,
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							int32_t				aTicksInState,
							ComponentBase**		aComponents,
							Context*			aContext) override;
		void			UpdatePublic(
							uint32_t			aEntityId,
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							int32_t				aTicksInState,
							ComponentBase**		aComponents,
							Context*			aContext) override;
	private:
	};

}