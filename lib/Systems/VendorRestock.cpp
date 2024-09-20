#include "../Pcheader.h"

#include <tpublic/Components/VendorStock.h>

#include <tpublic/Systems/VendorRestock.h>

namespace tpublic::Systems
{

	VendorRestock::VendorRestock(
		const SystemData*	aData)
		: SystemBase(aData, 100)
	{
		RequireComponent<Components::VendorStock>();
	}
	
	VendorRestock::~VendorRestock()
	{

	}

	//---------------------------------------------------------------------------

	void
	VendorRestock::UpdatePublic(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			aContext) 
	{
		Components::VendorStock* vendorStock = GetComponent<Components::VendorStock>(aComponents);

		for(Components::VendorStock::Item& item : vendorStock->m_itemList)
		{
			if(aContext->m_tick >= item.m_nextRestockTick)
			{
				if(item.m_current < item.m_max)
				{
					item.m_current++;
					vendorStock->SetDirty();
				}

				item.m_nextRestockTick = aContext->m_tick + item.m_restockTicks;
			}
		}
	}

}