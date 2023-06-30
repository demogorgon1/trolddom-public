#pragma once

#include "../Component.h"
#include "../EquipmentSlot.h"
#include "../ItemInstance.h"

namespace tpublic
{

	namespace Components
	{

		struct EquippedItems
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_EQUIPPED_ITEMS;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;

			EquippedItems()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
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
					m_slots[i].ToStream(aStream);
				aStream->WriteUInt(m_version);
			}

			bool
			FromStream(
				IReader*	aStream) override
			{
				for (uint32_t i = 0; i < (uint32_t)EquipmentSlot::NUM_IDS; i++)
				{
					if(!m_slots[i].FromStream(aStream))
						return false;
				}
				if(!aStream->ReadUInt(m_version))
					return false;
				return true;
			}

			// Public data
			ItemInstance	m_slots[EquipmentSlot::NUM_IDS];
			uint32_t		m_version = 0;
		};
	}

}