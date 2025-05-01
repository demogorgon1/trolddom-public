#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
{

	class RandomItemVendor
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_RANDOM_ITEM_VENDOR;

						RandomItemVendor(
							const SystemData*	aData);
		virtual			~RandomItemVendor();

		// SystemBase implementation
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