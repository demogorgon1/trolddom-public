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

			VendorBuyback()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~VendorBuyback()
			{

			}

			void
			AddItem(
				const ItemInstance&		aItemInstance)
			{
				m_version++;

				for (size_t i = 0; i < COUNT; i++)
				{
					if(!m_items[i].IsSet())
					{
						m_items[i] = aItemInstance;
						m_nextIndex = (i + 1) % COUNT;
						return;
					}
				}

				if(m_nextIndex < COUNT)
					m_items[m_nextIndex] = aItemInstance;
				m_nextIndex = (m_nextIndex + 1) % COUNT;
			}

			bool
			HasItems() const
			{
				for (size_t i = 0; i < COUNT; i++)
				{
					if(m_items[i].IsSet())
						return true;
				}
				return false;
			}

			void
			Clear()
			{
				for (size_t i = 0; i < COUNT; i++)
					m_items[i].Clear();
				m_nextIndex = 0;
				m_version++;
			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* aStream) const override
			{
				for(size_t i = 0; i < COUNT; i++)
					m_items[i].ToStream(aStream);

				aStream->WriteUInt(m_nextIndex);
				aStream->WriteUInt(m_version);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				for (size_t i = 0; i < COUNT; i++)
				{
					if(!m_items[i].FromStream(aStream))
						return false;
				}

				if(!aStream->ReadUInt(m_nextIndex))
					return false;
				if (!aStream->ReadUInt(m_version))
					return false;
				return true;
			}

			// Public data
			static const size_t COUNT = 4;
			ItemInstance	m_items[COUNT];
			size_t			m_nextIndex = 0;
			uint32_t		m_version = 0;
		};
	}

}