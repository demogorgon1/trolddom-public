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
				const ItemInstance&		aItemInstance)
			{
				for(Entry& t : m_entries)
				{
					if(!t.m_item.IsSet())
					{
						t.m_item = aItemInstance;
						m_version++;
						return true;
					}
				}
				return false;
			}

			bool
			Move(
				uint32_t				aSourceIndex,
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
						tpublic::ItemInstance temp = destination.m_item;
						destination.m_item = source.m_item;
						source.m_item = temp;
					}
					else
					{
						destination.m_item = source.m_item;
						source.m_item.Clear();
					}
				}

				return true;
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