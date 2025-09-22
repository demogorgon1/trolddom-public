#pragma once

#include "EquipmentSlot.h"
#include "ItemType.h"
#include "Rarity.h"
#include "SourceNode.h"

namespace tpublic
{

	struct TargetItemRequirements
	{
		TargetItemRequirements()
		{

		}

		TargetItemRequirements(
			const SourceNode*		aSource)
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "equipment_slots")
					aChild->GetIdArrayWithLookup<EquipmentSlot::Id, EquipmentSlot::INVALID_ID>(m_equipmentSlots, [](const char* aString) { return EquipmentSlot::StringToId(aString); });
				else if (aChild->m_name == "rarities")
					aChild->GetIdArrayWithLookup<Rarity::Id, Rarity::INVALID_ID>(m_rarities, [](const char* aString) { return Rarity::StringToId(aString); });
				else if (aChild->m_name == "item_types")
					aChild->GetIdArrayWithLookup<ItemType::Id, ItemType::INVALID_ID>(m_itemTypes, [](const char* aString) { return ItemType::StringToId(aString); });
				else if(aChild->m_name == "must_be_sellable")
					m_mustBeSellable = aChild->GetBool();
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
		}

		void	
		ToStream(
			IWriter*				aStream) const 
		{
			aStream->WritePODs(m_equipmentSlots);
			aStream->WritePODs(m_rarities);
			aStream->WritePODs(m_itemTypes);
			aStream->WriteBool(m_mustBeSellable);
		}
			
		bool	
		FromStream(
			IReader*				aStream) 
		{
			if (!aStream->ReadPODs(m_equipmentSlots))
				return false;
			if (!aStream->ReadPODs(m_rarities))
				return false;
			if (!aStream->ReadPODs(m_itemTypes))
				return false;
			if(!aStream->ReadBool(m_mustBeSellable))
				return false;
			return true;
		}

		// Public data
		std::vector<EquipmentSlot::Id>		m_equipmentSlots;
		std::vector<Rarity::Id>				m_rarities;
		std::vector<ItemType::Id>			m_itemTypes;
		bool								m_mustBeSellable = false;
	};

}