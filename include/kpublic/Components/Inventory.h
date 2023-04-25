#pragma once

#include "../Component.h"
#include "../ItemInstance.h"

namespace kpublic
{

	namespace Components
	{

		struct Inventory
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_INVENTORY;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_PLAYER_ONLY;

			struct Entry
			{	
				void
				ToStream(
					IWriter*	aStream) const 
				{
					aStream->WriteUInt(m_index);
					aStream->WritePOD(m_item);
				}

				bool
				FromStream(
					IReader*	aStream) 
				{
					if(!aStream->ReadUInt(m_index))
						return false;
					if (!aStream->ReadPOD(m_item))
						return false;
					return true;
				}

				// Public data
				uint32_t					m_index = 0;
				ItemInstance				m_item;
			};

			Inventory()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual
			~Inventory()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter*	aStream) const override
			{
				aStream->WriteObjects(m_entries);
			}

			bool
			FromStream(
				IReader*	aStream) override
			{
				if(!aStream->ReadObjects(m_entries))
					return false;
				return true;
			}

			// Public data
			std::vector<Entry>				m_entries;
		};
	}

}