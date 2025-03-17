#pragma once

#include "../ComponentBase.h"
#include "../ItemList.h"

namespace tpublic
{

	namespace Components
	{

		struct PlayerWorldStash
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_WORLD_STASH;
			static const uint8_t FLAGS = FLAG_REPLICATE_ONLY_ON_REQUEST | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_PLAYER_WORLD_STASH;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			static const uint32_t SOFT_SIZE_LIMIT = 32;

			struct Items
			{
				void
				ToStream(
					IWriter*						aWriter) const
				{
					aWriter->WriteUInt(m_table.size());
					for(Table::const_iterator i = m_table.cbegin(); i != m_table.cend(); i++)
					{	
						aWriter->WriteUInt(i->first);
						i->second->ToStream(aWriter);
					}
				}

				bool
				FromStream(
					IReader*						aReader) 
				{
					size_t count;
					if(!aReader->ReadUInt(count))
						return false;
					for(size_t i = 0; i < count; i++)
					{
						uint32_t playerWorldCharacterId;
						if (!aReader->ReadUInt(playerWorldCharacterId))
							return false;
						std::unique_ptr<ItemList> t = std::make_unique<ItemList>();
						if(!t->FromStream(aReader))
							return false;
						m_table[playerWorldCharacterId] = std::move(t);
					}
					return true;
				}

				ItemList*
				GetOrCreateItemList(
					uint32_t						aPlayerWorldCharacterId)
				{
					Table::iterator i = m_table.find(aPlayerWorldCharacterId);
					if(i != m_table.end())
						return i->second.get();
					ItemList* t = new ItemList();
					m_table[aPlayerWorldCharacterId] = std::unique_ptr<ItemList>(t);
					return t;
				}

				const ItemList*
				GetItemList(
					uint32_t		aPlayerWorldCharacterId) const
				{
					Table::const_iterator i = m_table.find(aPlayerWorldCharacterId);
					if(i == m_table.cend())
						return NULL;
					return i->second.get();
				}

				// Public data
				typedef std::unordered_map<uint32_t, std::unique_ptr<ItemList>> Table;
				Table		m_table;
			};

			enum Field : uint32_t
			{
				FIELD_ITEMS
			};

			static void
			CreateSchema(
				ComponentSchema*					aSchema)
			{
				aSchema->DefineCustomObjectNoSource<Items>(FIELD_ITEMS, offsetof(PlayerWorldStash, m_items));

				aSchema->OnRead<PlayerWorldStash>([](
					PlayerWorldStash*				aPlayerWorldStash,
					ComponentSchema::ReadType		aReadType,
					const Manifest*					aManifest)
				{
					if(aReadType == ComponentSchema::READ_TYPE_STORAGE)
					{
						for(Items::Table::iterator i = aPlayerWorldStash->m_items.m_table.begin(); i != aPlayerWorldStash->m_items.m_table.end(); i++)
							i->second->OnLoadedFromPersistence(aManifest, SOFT_SIZE_LIMIT);
					}
				});
			}

			uint32_t
			MoveWorldboundItemsToStash(
				const Manifest*						aManifest,
				ItemList&							aSourceItemList,
				ItemInstance						aSourceEquippedItems[EquipmentSlot::NUM_IDS])
			{
				uint32_t count = 0;

				for(ItemList::Entry& entry : aSourceItemList.m_entries)
				{
					if(entry.m_item.IsSet() && entry.m_item.IsWorldbound())	
					{
						TP_CHECK(!entry.m_trading, "Item in player world stash flagged for trading.");

						ItemList* destinationItemList = m_items.GetOrCreateItemList(entry.m_item.GetWorldboundCharacterId());

						if(destinationItemList->AddToInventory(entry.m_item, aManifest->GetById<Data::Item>(entry.m_item.m_itemId), true))
							count += entry.m_item.m_quantity;

						entry.m_item.Clear();
					}
				}

				for(uint32_t i = 0; i < (uint32_t)EquipmentSlot::NUM_IDS; i++)
				{
					ItemInstance& item = aSourceEquippedItems[i];
					if(item.IsSet() && item.IsWorldbound())
					{
						ItemList* destinationItemList = m_items.GetOrCreateItemList(item.GetWorldboundCharacterId());

						if (destinationItemList->AddToInventory(item, aManifest->GetById<Data::Item>(item.m_itemId), true))
							count += item.m_quantity;

						item.Clear();
					}
				}

				if(count > 0)
					aSourceItemList.m_version++;

				return count;
			}

			void
			Reset()
			{
				m_items.m_table.clear();
			}

			// Public data
			Items			m_items;
		};
	}

}