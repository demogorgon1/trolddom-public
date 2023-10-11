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

			enum Field
			{
				FIELD_ITEM_LIST
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->DefineCustomObjectNoSource<ItemList>(FIELD_ITEM_LIST, offsetof(Inventory, m_itemList));
			}

			void
			UpdateItemList(
				const ItemList&								aItemList)
			{
				uint32_t version = m_itemList.m_version + 1;
				m_itemList = aItemList;
				m_itemList.m_version = version;
			}

			// Public data
			ItemList						m_itemList;
		};
	}

}