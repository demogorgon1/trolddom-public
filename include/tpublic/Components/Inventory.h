#pragma once

#include "../Component.h"
#include "../ItemList.h"

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

			Inventory()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~Inventory()
			{

			}

			void
			UpdateItemList(
				const ItemList&								aItemList)
			{
				uint32_t version = m_itemList.m_version + 1;
				m_itemList = aItemList;
				m_itemList.m_version = version;
			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter*									aStream) const override
			{
				m_itemList.ToStream(aStream);
			}

			bool
			FromStream(
				IReader*									aStream) override
			{
				if(!m_itemList.FromStream(aStream))
					return false;
				return true;
			}

			// Public data
			ItemList						m_itemList;
		};
	}

}