#include "../Pcheader.h"

#include <tpublic/Components/RandomItemVendor.h>

#include <tpublic/Data/LootTable.h>

#include <tpublic/Systems/RandomItemVendor.h>

#include <tpublic/LootGenerator.h>
#include <tpublic/Manifest.h>

namespace tpublic::Systems
{

	RandomItemVendor::RandomItemVendor(
		const SystemData*	aData)
		: SystemBase(aData, 40)
	{
		RequireComponent<Components::RandomItemVendor>();
	}
	
	RandomItemVendor::~RandomItemVendor()
	{

	}

	//---------------------------------------------------------------------------

	void
	RandomItemVendor::UpdatePublic(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			aContext) 
	{
		Components::RandomItemVendor* randomItemVendor = GetComponent<Components::RandomItemVendor>(aComponents);

		if(randomItemVendor->m_items.size() != (size_t)randomItemVendor->m_itemCount)
			randomItemVendor->m_items.resize((size_t)randomItemVendor->m_itemCount);

		std::vector<Components::RandomItemVendor::Item*> refreshItems;
		bool dirty = false;

		for(Components::RandomItemVendor::Item& item : randomItemVendor->m_items)
		{
			if(item.m_itemId != 0)
			{
				if (aContext->m_tick > item.m_tick)
				{
					// Lose the item and set tick to when we should add another item
					item.m_itemId = 0;
					item.m_tick = aContext->m_tick + randomItemVendor->m_refreshTicks;
					
					dirty = true;
				}
			}
			else if(aContext->m_tick > item.m_tick)
			{
				// Need to refresh this item
				refreshItems.push_back(&item);

				dirty = true;
			}
		}

		if(refreshItems.size() > 0)
		{
			const Data::LootTable* lootTable = GetManifest()->GetById<Data::LootTable>(randomItemVendor->m_lootTableId);

			while(refreshItems.size() > 0)
			{
				GetData()->m_lootGenerator->GenerateItems(
					*aContext->m_random,
					std::vector<const EntityInstance*>(),
					NULL,
					randomItemVendor->m_level,
					0,
					false,
					lootTable,
					[&](
						const ItemInstance& aItemInstance,
						uint32_t			/*aLootCooldownId*/)
				{
					if(refreshItems.size() > 0 && !randomItemVendor->HasItem(aItemInstance.m_itemId))
					{				
						Components::RandomItemVendor::Item* refreshItem = refreshItems[refreshItems.size() - 1];
						refreshItems.pop_back();

						// Add the item and set the tick to when we should replace it
						refreshItem->m_itemId = aItemInstance.m_itemId;
						refreshItem->m_tick = aContext->m_tick + randomItemVendor->m_replacementTicks;
					}
				});
			}
		}

		if(dirty)
		{	
			randomItemVendor->m_version++;
			randomItemVendor->SetDirty();
		}
	}

}