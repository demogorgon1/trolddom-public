#pragma once

#include "../Component.h"

namespace tpublic
{

	namespace Components
	{

		struct VendorBuyback
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_VENDOR_BUYBACK;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;

			struct Items
			{
				void
				ToStream(
					IWriter*			aWriter) const
				{
					for (size_t i = 0; i < COUNT; i++)
						m_items[i].ToStream(aWriter);

					aWriter->WriteUInt(m_nextIndex);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					for (size_t i = 0; i < COUNT; i++)
					{
						if (!m_items[i].FromStream(aReader))
							return false;
					}
					if (!aReader->ReadUInt(m_nextIndex))
						return false;
					return true;
				}

				// Public data
				static const size_t COUNT = 4;
				ItemInstance	m_items[COUNT];
				size_t			m_nextIndex = 0;
			};

			enum Field
			{
				FIELD_ITEMS,
				FIELD_VERSION				
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectNoSource<Items>(FIELD_ITEMS, offsetof(VendorBuyback, m_items));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_VERSION, NULL, offsetof(VendorBuyback, m_version));
			}

			void
			AddItem(
				const ItemInstance&		aItemInstance)
			{
				m_version++;

				for (size_t i = 0; i < Items::COUNT; i++)
				{
					if(!m_items.m_items[i].IsSet())
					{
						m_items.m_items[i] = aItemInstance;
						m_items.m_nextIndex = (i + 1) % Items::COUNT;
						return;
					}
				}

				if(m_items.m_nextIndex < Items::COUNT)
					m_items.m_items[m_items.m_nextIndex] = aItemInstance;
				m_items.m_nextIndex = (m_items.m_nextIndex + 1) % Items::COUNT;
			}

			bool
			HasItems() const
			{
				for (size_t i = 0; i < Items::COUNT; i++)
				{
					if(m_items.m_items[i].IsSet())
						return true;
				}
				return false;
			}

			void
			Clear()
			{
				for (size_t i = 0; i < Items::COUNT; i++)
					m_items.m_items[i].Clear();
				m_items.m_nextIndex = 0;
				m_version++;
			}

			// Public data
			Items			m_items;
			uint32_t		m_version = 0;
		};
	}

}