#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"
#include "UIntCurve.h"

namespace tpublic
{

	class AbilityMetrics
	{
	public:		
		void
		FromSource(
			const SourceNode*			aSource)
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "training_cost_at_skill")
					m_trainingCostAtSkillCurve = UIntCurve<uint32_t>(aChild);
				else if (aChild->m_name == "rage_constant_at_level")
					m_rageConstantAtLevelCurve = UIntCurve<uint32_t>(aChild);
				else if (aChild->m_name == "untrain_talents_cost_at_level")
					m_untrainTalentsCostAtLevelCurve = UIntCurve<uint32_t>(aChild);
				else if (aChild->m_name == "rage_hit_base_per_second")
					m_rageHitBasePerSecond = aChild->GetUInt32();
				else if (aChild->m_name == "rage_crit_base_per_second")
					m_rageCritBasePerSecond = aChild->GetUInt32();
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
		}

		void
		ToStream(
			IWriter*						aStream) const 
		{
			m_trainingCostAtSkillCurve.ToStream(aStream);
			m_rageConstantAtLevelCurve.ToStream(aStream);
			m_untrainTalentsCostAtLevelCurve.ToStream(aStream);
			aStream->WriteUInt(m_rageHitBasePerSecond);
			aStream->WriteUInt(m_rageCritBasePerSecond);
		}

		bool
		FromStream(
			IReader*						aStream) 
		{
			if (!m_trainingCostAtSkillCurve.FromStream(aStream))
				return false;
			if (!m_rageConstantAtLevelCurve.FromStream(aStream))
				return false;
			if (!m_untrainTalentsCostAtLevelCurve.FromStream(aStream))
				return false;
			if (!aStream->ReadUInt(m_rageHitBasePerSecond))
				return false;
			if (!aStream->ReadUInt(m_rageCritBasePerSecond))
				return false;
			return true;
		}
				
		// Public data
		UIntCurve<uint32_t>		m_trainingCostAtSkillCurve;
		UIntCurve<uint32_t>		m_rageConstantAtLevelCurve;
		uint32_t				m_rageHitBasePerSecond = 0;
		uint32_t				m_rageCritBasePerSecond = 0;
		UIntCurve<uint32_t>		m_untrainTalentsCostAtLevelCurve;
	};

}
