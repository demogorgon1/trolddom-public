#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"

namespace tpublic
{

	class MiscMetrics
	{
	public:		
		void
		FromSource(
			const SourceNode*			aSource)
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "first_extra_stash_slot_price")
					m_firstExtraStashSlotPrice = aChild->GetInt64();
				else if (aChild->m_name == "max_extra_stash_slot_price")
					m_maxExtraStashSlotPrice = aChild->GetInt64();
				else if (aChild->m_name == "max_extra_stash_slots")
					m_maxExtraStashSlots = aChild->GetUInt32();
				else if (aChild->m_name == "additional_extra_stash_slot_price_multiplier")
					m_additionalExtraStashSlotPriceMultiplier = aChild->GetFloat();
				else if(aChild->m_name == "demo_maps")
					aChild->GetIdSet(DataType::ID_MAP, m_demoMaps);
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
		}

		void
		ToStream(
			IWriter*						aStream) const 
		{
			aStream->WriteInt(m_firstExtraStashSlotPrice);
			aStream->WriteInt(m_maxExtraStashSlotPrice);
			aStream->WriteUInt(m_maxExtraStashSlots);
			aStream->WriteFloat(m_additionalExtraStashSlotPriceMultiplier);
			aStream->WriteUIntSet(m_demoMaps);
		}

		bool
		FromStream(
			IReader*						aStream) 
		{
			if (!aStream->ReadInt(m_firstExtraStashSlotPrice))
				return false;
			if (!aStream->ReadInt(m_maxExtraStashSlotPrice))
				return false;
			if (!aStream->ReadUInt(m_maxExtraStashSlots))
				return false;
			if (!aStream->ReadFloat(m_additionalExtraStashSlotPriceMultiplier))
				return false;
			if (!aStream->ReadUIntSet(m_demoMaps))
				return false;
			return true;
		}

		int64_t
		GetNextExtraStashSlotPrice(
			uint32_t						aCurrentExtraStashSlots) const
		{
			if(aCurrentExtraStashSlots >= m_maxExtraStashSlots)
				return 0;

			if(m_additionalExtraStashSlotPriceMultiplier == 0.0f)
				return 0;

			int64_t price = (int64_t)(m_firstExtraStashSlotPrice * powf(m_additionalExtraStashSlotPriceMultiplier, (float)aCurrentExtraStashSlots));
			if(price > m_maxExtraStashSlotPrice)
				price = m_maxExtraStashSlotPrice;

			return price;
		}
				
		// Public data
		int64_t							m_firstExtraStashSlotPrice = 0;
		int64_t							m_maxExtraStashSlotPrice = 0;
		uint32_t						m_maxExtraStashSlots = 0;
		float							m_additionalExtraStashSlotPriceMultiplier = 0.0f;
		std::unordered_set<uint32_t>	m_demoMaps;
	};

}
