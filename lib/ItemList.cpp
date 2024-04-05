#include "Pcheader.h"

#include <tpublic/ItemList.h>

namespace tpublic
{

	void				
	ItemList::ToStream(
		IWriter*									aWriter) const
	{
		aWriter->WriteObjects(m_entries);
		aWriter->WriteUInt(m_version);
	}
	
	bool				
	ItemList::FromStream(
		IReader*									aReader)
	{
		if(!aReader->ReadObjects(m_entries))
			return false;
		if(!aReader->ReadUInt(m_version))
			return false;
		return true;
	}

	bool
	ItemList::AddToInventory(
		const ItemInstance&							aItemInstance,
		const Data::Item*							aItemData,
		bool										aAutoGrow)
	{
		if(aItemData->m_unique && HasItems(aItemInstance.m_itemId, 1))
			return false;

		for(Entry& t : m_entries)
		{
			if(!t.m_item.IsSet())
			{
				t.m_item = aItemInstance;
				m_version++;
				return true;
			}
			else if(t.m_item.IsSet() && t.m_item.m_itemId == aItemInstance.m_itemId)
			{
				assert(t.m_item.m_itemId == aItemData->m_id);

				if(t.m_item.m_quantity + aItemInstance.m_quantity <= aItemData->m_stackSize)
				{
					t.m_item.m_quantity += aItemInstance.m_quantity;
					m_version++;
					return true;
				}
			}
		}

		if(aAutoGrow)
		{
			Entry t;
			t.m_item = aItemInstance;
			m_entries.push_back(t);
			m_version++;
			return true;
		}

		return false;
	}

	bool
	ItemList::CanAddMultipleToInventory(
		uint32_t									aItemId,
		uint32_t									aQuantity,
		const Data::Item*							aItemData) const
	{
		if (aItemData->m_unique && HasItems(aItemId, 1))
			return false;

		if(aQuantity == 0)
			return true;

		uint32_t remaining = aQuantity;
				
		for (const Entry& t : m_entries)
		{
			if(!t.m_item.IsSet())
			{
				uint32_t toAdd = remaining;
				if(toAdd > aItemData->m_stackSize)
					toAdd = aItemData->m_stackSize;  

				remaining -= toAdd;
			}
			else if(t.m_item.IsSet() && t.m_item.m_itemId == aItemId && t.m_item.m_quantity < aItemData->m_stackSize)
			{
				uint32_t toAdd = remaining;
				if (toAdd > aItemData->m_stackSize - t.m_item.m_quantity)
					toAdd = aItemData->m_stackSize - t.m_item.m_quantity;

				remaining -=  toAdd;
			}

			if(remaining == 0)
				return true;
		}

		return false;
	}

	void
	ItemList::AddMultipleToInventory(
		uint32_t									aItemId,
		uint32_t									aQuantity,
		const Data::Item*							aItemData) 
	{
		if (aItemData->m_unique && HasItems(aItemId, 1))
			return;

		if(aQuantity == 0)
			return;

		uint32_t remaining = aQuantity;

		for (Entry& t : m_entries)
		{
			if (!t.m_item.IsSet())
			{
				uint32_t toAdd = remaining;
				if (toAdd > aItemData->m_stackSize)
					toAdd = aItemData->m_stackSize;

				remaining -= toAdd;

				ItemInstance itemInstance;
				itemInstance.m_itemId = aItemId;
				itemInstance.m_quantity = toAdd;
						
				t.m_item = itemInstance;
			}
			else if (t.m_item.IsSet() && t.m_item.m_itemId == aItemId && t.m_item.m_quantity < aItemData->m_stackSize)
			{
				uint32_t toAdd = remaining;
				if (toAdd > aItemData->m_stackSize - t.m_item.m_quantity)
					toAdd = aItemData->m_stackSize - t.m_item.m_quantity;

				remaining -= toAdd;

				t.m_item.m_quantity += toAdd;
			}

			if (remaining == 0)
				break;
		}

		m_version++;

		assert(remaining == 0);
	}

	bool
	ItemList::Destroy(
		uint32_t									aIndex,
		const Data::Item*							aItemData,
		uint32_t									aQuantity)
	{
		if ((size_t)aIndex >= m_entries.size() || (size_t)aIndex >= m_entries.size())
			return false;

		Entry& entry = m_entries[aIndex];

		if(!entry.m_item.IsSet())
			return false;

		if(entry.m_item.m_itemId != aItemData->m_id)
			return false;

		uint32_t quantity = aQuantity;
		if(quantity == 0 || quantity > entry.m_item.m_quantity)
			quantity = entry.m_item.m_quantity;

		entry.m_item.m_quantity -= quantity;

		if(entry.m_item.m_quantity == 0)
			entry.m_item.Clear();
					
		m_version++;

		return true;
	}

	bool
	ItemList::HasItems(
		uint32_t									aItemId,
		uint32_t									aQuantityRequired) const
	{
		uint32_t count = 0;

		for(const Entry& t : m_entries)
		{
			if(t.m_item.IsSet() && t.m_item.m_itemId == aItemId)
			{
				count += t.m_item.m_quantity;

				if(count >= aQuantityRequired)
					return true;
			}
		}

		return false;
	}

	void
	ItemList::RemoveItems(
		uint32_t									aItemId,
		uint32_t									aQuantity)
	{
		uint32_t remaining = aQuantity;

		for (Entry& t : m_entries)
		{
			if(remaining == 0)
				break;

			if (t.m_item.IsSet() && t.m_item.m_itemId == aItemId)
			{
				uint32_t toRemove = remaining;
				if(toRemove >= t.m_item.m_quantity)
					toRemove = t.m_item.m_quantity;

				t.m_item.m_quantity -= toRemove;
				remaining -= toRemove;

				if(t.m_item.m_quantity == 0)
					t.m_item.Clear();						
			}
		}

		assert(remaining == 0);
		m_version++;
	}

	bool
	ItemList::Consume(
		const Data::Ability::Items*					aConsumeItems)
	{
		// First pass to see if we actually have everything
		if(!CanConsume(aConsumeItems))
			return false;

		// Second pass we remove stuff
		for (const Data::Ability::Item& item : aConsumeItems->m_items)
			RemoveItems(item.m_itemId, item.m_quantity);

		return true;
	}

	bool
	ItemList::CanConsume(
		const Data::Ability::Items*					aConsumeItems) const
	{
		if (aConsumeItems == NULL)
			return true;

		for (const Data::Ability::Item& item : aConsumeItems->m_items)
		{
			if (!HasItems(item.m_itemId, item.m_quantity))
				return false;
		}

		return true;
	}

	bool
	ItemList::Move(
		uint32_t									aSourceIndex,
		const Data::Item*							aSourceItemData,
		uint32_t									aDestinationIndex,
		uint32_t									aSplitQuantity)
	{
		if((size_t)aSourceIndex >= m_entries.size() || (size_t)aDestinationIndex >= m_entries.size())
			return false;

		if (aSourceIndex != aDestinationIndex)
		{
			Entry& source = m_entries[aSourceIndex];
			Entry& destination = m_entries[aDestinationIndex];

			if(!source.m_item.IsSet())
				return false;

			if(destination.m_item.IsSet())
			{
				bool doSwap = true;

				if (aSplitQuantity != 0)
				{
					if(source.m_item.m_itemId != destination.m_item.m_itemId)
						return false;

					if (aSplitQuantity >= source.m_item.m_quantity)
						return false;

					uint32_t quantityToMove = aSplitQuantity;
					if(destination.m_item.m_quantity + quantityToMove > aSourceItemData->m_stackSize)
						quantityToMove = aSourceItemData->m_stackSize - destination.m_item.m_quantity;

					destination.m_item.m_quantity += quantityToMove;
					source.m_item.m_quantity -= quantityToMove;
					doSwap = false;
				}
				else
				{
					if(source.m_item.m_itemId == destination.m_item.m_itemId)
					{
						assert(aSourceItemData->m_id == source.m_item.m_itemId);

						if(destination.m_item.m_quantity + source.m_item.m_quantity <= aSourceItemData->m_stackSize)
						{
							// Destination got enough space for everything
							destination.m_item.m_quantity += source.m_item.m_quantity;
							source.m_item.Clear();
							doSwap = false;
						}
						else if(destination.m_item.m_quantity < aSourceItemData->m_stackSize)
						{
							// Only move some to destination
							uint32_t quantityToMove = aSourceItemData->m_stackSize - destination.m_item.m_quantity;
							destination.m_item.m_quantity += quantityToMove;
							assert(source.m_item.m_quantity > quantityToMove);
							source.m_item.m_quantity -= quantityToMove;
							doSwap = false;
						}
					}
				}
						
				if(doSwap)
				{
					tpublic::ItemInstance temp = destination.m_item;
					destination.m_item = source.m_item;
					source.m_item = temp;
				}
			}
			else
			{
				if (aSplitQuantity != 0)
				{
					if (aSplitQuantity >= source.m_item.m_quantity)
						return false;

					destination.m_item = source.m_item;
					destination.m_item.m_quantity = aSplitQuantity;
					source.m_item.m_quantity -= aSplitQuantity;
				}
				else
				{
					destination.m_item = source.m_item;
					source.m_item.Clear();
				}
			}

			m_version++;
		}

		return true;
	}

	const ItemInstance*
	ItemList::GetItemAtIndex(
		uint32_t									aIndex) const
	{
		if((size_t)aIndex < m_entries.size())
			return &m_entries[aIndex].m_item;
		return NULL;
	}

	ItemInstance*
	ItemList::GetItemAtIndex(
		uint32_t									aIndex) 
	{
		if((size_t)aIndex < m_entries.size())
			return &m_entries[aIndex].m_item;
		return NULL;
	}
			
	void
	ItemList::ForEach(
		std::function<void(const ItemInstance&)>	aCallback) const
	{
		for(const Entry& t : m_entries)
		{
			if(t.m_item.IsSet())
				aCallback(t.m_item);
		}
	}

	size_t
	ItemList::GetSpace() const
	{
		size_t count = 0;
		for (const Entry& t : m_entries)
		{
			if(!t.m_item.IsSet())
				count++;
		}
		return count;
	}	

	size_t				
	ItemList::GetItemCount(
		uint32_t									aItemId) const
	{
		size_t count = 0;
		for (const Entry& t : m_entries)
		{
			if(t.m_item.IsSet() && t.m_item.m_itemId == aItemId)
				count += (size_t)t.m_item.m_quantity;
		}

		return count;
	}

}

