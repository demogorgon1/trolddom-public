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
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
		}

		void
		ToStream(
			IWriter*						aStream) const 
		{
			m_trainingCostAtSkillCurve.ToStream(aStream);
		}

		bool
		FromStream(
			IReader*						aStream) 
		{
			if(!m_trainingCostAtSkillCurve.FromStream(aStream))
				return false;
			return true;
		}
				
		// Public data
		UIntCurve<uint32_t>		m_trainingCostAtSkillCurve;
	};

}
