#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"

namespace tpublic
{

	class FloatRange
	{
	public:
		FloatRange()
		{

		}

		FloatRange(
			float					aValue) 
			: m_min(aValue)
			, m_max(aValue)
		{

		}

		FloatRange(
			float					aMin,
			float					aMax)
			: m_min(aMin)
			, m_max(aMax)
		{

		}

		FloatRange(
			const SourceNode*		aSource)
		{
			if(aSource->m_type == SourceNode::TYPE_ARRAY)
			{
				if(aSource->m_children.size() == 2)
				{
					m_min = aSource->m_children[0]->GetFloat();
					m_max = aSource->m_children[1]->GetFloat();
				}
				else
				{
					TP_VERIFY(aSource->m_children.size() == 1, aSource->m_debugInfo, "Not a valid range.");
					m_min = aSource->m_children[0]->GetFloat();
					m_max = m_min;
				}
			}
			else
			{
				m_min = aSource->GetFloat();
				m_max = m_min;
			}

			TP_VERIFY(m_min <= m_max, aSource->m_debugInfo, "Minimum larger than maximum.");
		}

		void
		ToStream(
			IWriter*				aWriter) const
		{
			aWriter->WriteFloat(m_min);
			aWriter->WriteFloat(m_max);
		}

		bool
		FromStream(
			IReader*				aReader)
		{
			if (!aReader->ReadFloat(m_min))
				return false;
			if (!aReader->ReadFloat(m_max))
				return false;
			if(m_min > m_max)
				return false;
			return true;
		}

		float
		GetRandom(
			std::mt19937&			aRandom) const
		{
			assert(m_min <= m_max);
			std::uniform_real_distribution<float> distribution(m_min, m_max);
			return distribution(aRandom);
		}

		// Public data
		float	m_min = 0;
		float	m_max = 0;
	};

}