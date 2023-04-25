#pragma once

#include "../Component.h"
#include "../EquipmentSlot.h"
#include "../ItemInstance.h"

namespace kpublic
{

	namespace Components
	{

		struct EquippedItems
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_EQUIPPED_ITEMS;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS | FLAG_PLAYER_ONLY | FLAG_PERSISTENT;

			EquippedItems()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual
			~EquippedItems()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter*	aStream) const override
			{
				for(uint32_t i = 0; i < (uint32_t)EquipmentSlot::NUM_IDS; i++)
					aStream->WritePOD(m_slots[i]);
			}

			bool
			FromStream(
				IReader*	aStream) override
			{
				for (uint32_t i = 0; i < (uint32_t)EquipmentSlot::NUM_IDS; i++)
				{
					if(!aStream->ReadPOD(m_slots[i]))
						return false;
				}
				return true;
			}

			// Public data
			ItemInstance	m_slots[EquipmentSlot::NUM_IDS];
		};
	}

}