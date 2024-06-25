#pragma once

#include "../Data/Item.h"

#include "../Component.h"
#include "../ComponentBase.h"
#include "../EquipmentSlot.h"
#include "../ItemInstance.h"
#include "../ItemType.h"
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

				bool
				HasEquippedSpecificItem(
					uint32_t			aItemId) const
				{
					for (uint32_t i = 0; i < (uint32_t)EquipmentSlot::NUM_IDS; i++)
					{
						if(m_items[i].m_itemId == aItemId)
							return true;
					}
					return false;
				}

				ArmorStyle::Visual
				GetArmorStyleVisual(
					const Manifest*		aManifest) const
				{
					const ItemInstance& chest = m_items[EquipmentSlot::ID_CHEST];
					if(chest.IsSet())
					{
						const Data::Item* itemData = aManifest->GetById<Data::Item>(chest.m_itemId);
						return itemData->GetArmorStyleVisual();
					}

					ArmorStyle::Visual none;
					none.m_id = ArmorStyle::ID_NONE;
					return none;
				}

				ItemType::Id
				GetMainHandItemType(
					const Manifest*		aManifest) const
				{
					const ItemInstance& mainHand = m_items[EquipmentSlot::ID_MAIN_HAND];
					if (mainHand.IsSet())
					{
						const Data::Item* itemData = aManifest->GetById<Data::Item>(mainHand.m_itemId);
						return itemData->m_itemType;
					}

					return ItemType::ID_NONE;
				}

				ItemType::Id
				GetOffHandItemType(
					const Manifest*		aManifest) const
				{
					const ItemInstance& offHand = m_items[EquipmentSlot::ID_OFF_HAND];
					if (offHand.IsSet())
					{
						const Data::Item* itemData = aManifest->GetById<Data::Item>(offHand.m_itemId);
						return itemData->m_itemType;
					}

					return ItemType::ID_NONE;
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