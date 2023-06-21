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