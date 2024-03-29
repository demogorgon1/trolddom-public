#pragma once

#include "../Component.h"
#include "../ComponentBase.h"
#include "../ItemList.h"

namespace tpublic
{

	namespace Components
	{

		struct Inventory
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_INVENTORY;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

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
			ItemList							m_itemList;

			// Not serialized
			std::vector<tpublic::ItemInstance>	m_itemLootNotifications;
		};
	}

}