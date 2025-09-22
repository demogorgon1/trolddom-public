#pragma once

#include "Image.h"
#include "IReader.h"
#include "ItemInstanceData.h"
#include "IWriter.h"
#include "SourceNode.h"

namespace tpublic
{

	class WorshipMetrics
	{
	public:		
		struct LevelColors
		{
			LevelColors()
			{

			}

			LevelColors(
				const SourceNode*		aSource)
			{
				m_color1 = Image::RGBA(aSource->GetArrayIndex(0));
				m_color2 = Image::RGBA(aSource->GetArrayIndex(1));
				m_color3 = Image::RGBA(aSource->GetArrayIndex(2));
			}

			void
			ToStream(
				IWriter*				aWriter) const
			{
				aWriter->WritePOD(m_color1);
				aWriter->WritePOD(m_color2);
				aWriter->WritePOD(m_color3);
			}

			bool
			FromStream(
				IReader*				aReader)
			{
				if (!aReader->ReadPOD(m_color1))
					return false;
				if (!aReader->ReadPOD(m_color2))
					return false;
				if (!aReader->ReadPOD(m_color3))
					return false;
				return true;
			}

			// Public data
			Image::RGBA			m_color1;
			Image::RGBA			m_color2;
			Image::RGBA			m_color3;
		};

		WorshipMetrics()
		{

		}

		void
		FromSource(
			const SourceNode*			aSource)
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "max_favor")
					m_maxFavor = aChild->GetInt32();
				else if (aChild->m_name == "min_favor")
					m_minFavor = aChild->GetInt32();
				else if (aChild->m_name == "disciple_level")
					m_discipleLevel = aChild->GetInt32();
				else if (aChild->m_name == "base_favor_update")
					m_baseFavorUpdate = aChild->GetInt32();
				else if (aChild->m_name == "favor_loss_multiplier")
					m_favorLossMultiplier = aChild->GetInt32();
				else if(aChild->m_name == "levels")
					aChild->GetUIntArray(m_levels);
				else if (aChild->m_name == "deity_power_rank_percentiles")
					aChild->GetUIntArray(m_deityPowerRankPercentiles);
				else if (aChild->m_name == "level_colors")
					aChild->GetObjectArray(m_levelColors);
				else if (aChild->m_name == "item_cost_to_favor_constant_a")
					m_itemCostToFavorConstantA = aChild->GetFloat();
				else if (aChild->m_name == "item_cost_to_favor_constant_b")
					m_itemCostToFavorConstantB = aChild->GetFloat();
				else if (aChild->m_name == "stackable_vendor_item_favor_multiplier")
					m_stackableVendorItemFavorMultiplier = aChild->GetFloat();
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
		}

		void
		ToStream(
			IWriter*						aStream) const 
		{
			aStream->WriteInt(m_maxFavor);
			aStream->WriteInt(m_minFavor);
			aStream->WriteInt(m_discipleLevel);
			aStream->WriteInt(m_baseFavorUpdate);
			aStream->WriteInt(m_favorLossMultiplier);
			aStream->WriteUInts(m_levels);
			aStream->WriteUInts(m_deityPowerRankPercentiles);
			aStream->WriteObjects(m_levelColors);
			aStream->WriteFloat(m_itemCostToFavorConstantA);
			aStream->WriteFloat(m_itemCostToFavorConstantB);
			aStream->WriteFloat(m_stackableVendorItemFavorMultiplier);
		}

		bool
		FromStream(
			IReader*						aStream) 
		{
			if (!aStream->ReadInt(m_maxFavor))
				return false;
			if (!aStream->ReadInt(m_minFavor))
				return false;
			if (!aStream->ReadInt(m_discipleLevel))
				return false;
			if (!aStream->ReadInt(m_baseFavorUpdate))
				return false;
			if (!aStream->ReadInt(m_favorLossMultiplier))
				return false;
			if(!aStream->ReadUInts(m_levels))
				return false;
			if (!aStream->ReadUInts(m_deityPowerRankPercentiles))
				return false;
			if(!aStream->ReadObjects(m_levelColors))
				return false;
			if(!aStream->ReadFloat(m_itemCostToFavorConstantA))
				return false;
			if (!aStream->ReadFloat(m_itemCostToFavorConstantB))
				return false;
			if (!aStream->ReadFloat(m_stackableVendorItemFavorMultiplier))
				return false;
			return true;
		}

		uint32_t
		GetLevelFromFavor(
			int32_t							aFavor) const
		{	
			if(aFavor < 0)
				return UINT32_MAX;

			uint32_t level = 0;
			for(uint32_t t : m_levels)
			{
				if((uint32_t)aFavor < t)					
					break;

				level++;
			}
			if(level > 0)
				level--;
			return level;
		}

		bool
		GetLevelFavorRange(
			uint32_t						aLevel,
			int32_t&						aOutMin,
			int32_t&						aOutMax) const
		{
			if(aLevel == UINT32_MAX)
			{
				aOutMin = m_minFavor;
				aOutMax = 0;
				return true;
			}

			if(aLevel >= (uint32_t)m_levels.size())
				return false;

			if(aLevel == (uint32_t)m_levels.size() - 1)
			{
				aOutMin = m_levels[aLevel - 1];
				aOutMax = m_maxFavor;
			}
			else 
			{
				aOutMin = m_levels[aLevel];
				aOutMax = m_levels[aLevel + 1];
			}

			return true;
		}

		const LevelColors&
		GetLevelColor(
			uint32_t						aLevel) const
		{
			TP_CHECK(m_levelColors.size() > 0, "No level colors defined.");
			uint32_t index = aLevel == UINT32_MAX ? 0 : aLevel + 1;
			if (index >= (uint32_t)m_levelColors.size())
				index = (uint32_t)m_levelColors.size() - 1;

			return m_levelColors[index];
		}

		uint32_t
		GetMaxLevel() const
		{	
			TP_CHECK(m_levels.size() > 0, "No levels defined.");
			return (uint32_t)m_levels.size() - 1;
		}

		int32_t 
		CalculateOfferingFavor(
			const ItemInstanceData&			aItemInstanceData,
			uint32_t						aStackSize) const
		{	
			float x = (float)aItemInstanceData.m_cost;
			if (aItemInstanceData.m_itemData->m_stackSize > 1 && aItemInstanceData.m_itemData->IsVendor())
				x *= m_stackableVendorItemFavorMultiplier;

			float y = (1.0f - 1.0f / ((x / m_itemCostToFavorConstantA) + 1.0f)) * (m_itemCostToFavorConstantB - 1.0f) + 1.0f;
			float favor = y * (float)aStackSize;
			return (int32_t)favor;
		}
				
		// Public data
		int32_t								m_maxFavor = 0;
		int32_t								m_minFavor = 0;
		int32_t								m_baseFavorUpdate = 0;
		int32_t								m_favorLossMultiplier = 0;
		int32_t								m_discipleLevel = 0;
		float								m_itemCostToFavorConstantA = 1.0f;
		float								m_itemCostToFavorConstantB = 1.0f;
		float								m_stackableVendorItemFavorMultiplier = 1.0f;
		std::vector<uint32_t>				m_levels;
		std::vector<LevelColors>			m_levelColors;
		std::vector<uint32_t>				m_deityPowerRankPercentiles;
	};

}
