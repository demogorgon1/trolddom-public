#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct VendorStock
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_VENDOR_STOCK;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			struct Item
			{
				void
				FromSource(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "item")
							m_itemId = aChild->GetId(DataType::ID_ITEM);
						else if (aChild->m_name == "count")
							m_max = aChild->GetUInt32();
						else if(aChild->m_name == "restock_ticks")
							m_restockTicks = aChild->GetInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});

					m_current = m_max;
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_itemId);
					aWriter->WriteUInt(m_current);
					aWriter->WriteUInt(m_max);
					aWriter->WriteInt(m_restockTicks);
					aWriter->WriteInt(m_nextRestockTick);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if (!aReader->ReadUInt(m_itemId))
						return false;
					if (!aReader->ReadUInt(m_current))
						return false;
					if (!aReader->ReadUInt(m_max))
						return false;
					if (!aReader->ReadInt(m_restockTicks))
						return false;
					if (!aReader->ReadInt(m_nextRestockTick))
						return false;
					return true;
				}

				// Public data
				uint32_t						m_itemId = 0;
				uint32_t						m_current = 0;
				uint32_t						m_max = 0;
				int32_t							m_restockTicks = 0;
				int32_t							m_nextRestockTick = 0;
			};

			enum Field
			{
				FIELD_ITEM_LIST
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->DefineCustomObjects<Item>(FIELD_ITEM_LIST, "item_list", offsetof(VendorStock, m_itemList));
			}

			void
			Reset()
			{
				m_itemList.clear();
			}

			uint32_t
			Get(
				uint32_t				aItemId) const
			{
				for(const Item& t : m_itemList)
				{
					if(t.m_itemId == aItemId)
						return t.m_current;
				}

				return UINT32_MAX;
			}

			bool
			Remove(
				uint32_t				aItemId,
				uint32_t				aQuantity)
			{
				for (Item& t : m_itemList)
				{
					if (t.m_itemId == aItemId)
					{
						if(aQuantity > t.m_current)
							return false;

						t.m_current -= aQuantity;
						return true;
					}
				}
				return false;
			}

			// Public data
			std::vector<Item>					m_itemList;
		};
	}

}