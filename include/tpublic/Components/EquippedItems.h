#pragma once

#include "../Data/Item.h"

#include "../Component.h"
#include "../ComponentBase.h"
#include "../EquipmentSlot.h"
#include "../ItemInstance.h"
#include "../Manifest.h"

namespace tpublic
{

	namespace Components
	{

		struct EquippedItems
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_EQUIPPED_ITEMS;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			struct Slots
			{
				void
				ToStream(
					IWriter*			aWriter) const
				{
					for (uint32_t i = 0; i < (uint32_t)EquipmentSlot::NUM_IDS; i++)
						m_items[i].ToStream(aWriter);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					for (uint32_t i = 0; i < (uint32_t)EquipmentSlot::NUM_IDS; i++)
					{
						if (!m_items[i].FromStream(aReader))
							return false;
					}
					return true;
				}

				bool 
				HasEquipped(
					const Manifest*		aManifest,
					EquipmentSlot::Id	aEquipmentSlotId,
					uint16_t			aFlags) const
				{
					const ItemInstance& item = m_items[aEquipmentSlotId];
					if(!item.IsSet())
						return false;

					const Data::Item* itemData = aManifest->GetById<Data::Item>(item.m_itemId);
					const ItemType::Info* itemTypeInfo = ItemType::GetInfo(itemData->m_itemType);
					return (itemTypeInfo->m_flags & aFlags) == aFlags;
				}

				// Public data
				ItemInstance	m_items[EquipmentSlot::NUM_IDS];
			};

			enum Field
			{
				FIELD_SLOTS,
				FIELD_VERSION
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->DefineCustomObjectNoSource<Slots>(FIELD_SLOTS, offsetof(EquippedItems, m_slots));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_VERSION, NULL, offsetof(EquippedItems, m_version));
			}

			void
			ForEach(
				std::function<void(const ItemInstance&)>	aCallback) const
			{
				for (uint32_t i = 0; i < (uint32_t)EquipmentSlot::NUM_IDS; i++)
				{
					const ItemInstance& t = m_slots.m_items[i];
					if(t.IsSet())
						aCallback(t);
				}
			}

			// Public data
			Slots			m_slots;
			uint32_t		m_version = 0;
		};
	}

}