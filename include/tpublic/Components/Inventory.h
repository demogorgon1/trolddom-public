#pragma once

#include "../Component.h"
#include "../ItemInstance.h"

namespace tpublic
{

	namespace Components
	{

		struct Inventory
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_INVENTORY;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;

			struct Entry
			{	
				void
				ToStream(
					IWriter*	aStream) const 
				{
					aStream->WritePOD(m_item);
				}

				bool
				FromStream(
					IReader*	aStream) 
				{
					if (!aStream->ReadPOD(m_item))
						return false;
					return true;
				}

				// Public data
				ItemInstance				m_item;
			};

			Inventory()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~Inventory()
			{

			}

			bool
			AddToInventory(
				const ItemInstance&		aItemInstance,
				const Data::Item*		aItemData)
			{
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
				return false;
			}

			bool
			Move(
				uint32_t				aSourceIndex,
				const Data::Item*		aSourceItemData,
				uint32_t				aDestinationIndex)
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
						
						if(doSwap)
						{
							tpublic::ItemInstance temp = destination.m_item;
							destination.m_item = source.m_item;
							source.m_item = temp;
						}
					}
					else
					{
						destination.m_item = source.m_item;
						source.m_item.Clear();
					}

					m_version++;
				}

				return true;
			}

			const ItemInstance*
			GetItemAtIndex(
				uint32_t				aIndex) const
			{
				if((size_t)aIndex < m_entries.size())
					return &m_entries[aIndex].m_item;
				return NULL;
			}
			
			// ComponentBase implementation
			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteObjects(m_entries);
				aStream->WriteUInt(m_version);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadObjects(m_entries))
					return false;
				if(!aStream->ReadUInt(m_version))
					return false;
				return true;
			}

			// Public data
			std::vector<Entry>				m_entries;
			uint32_t						m_version;
		};
	}

}