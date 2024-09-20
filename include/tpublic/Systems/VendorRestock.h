#pragma once

#include "../SystemBase.h"

namespace tpublic::Systems
{

	class VendorRestock
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_VENDOR_RESTOCK;

						VendorRestock(
							const SystemData*	aData);
		virtual			~VendorRestock();

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