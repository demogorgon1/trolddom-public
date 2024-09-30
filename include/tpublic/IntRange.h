#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"
#include "UniformDistribution.h"

namespace tpublic
{

	class IntRange
	{
	public:
		IntRange()
		{

		}

		IntRange(
			int32_t					aValue) 
			: m_min(aValue)
			, m_max(aValue)
		{

		}

		IntRange(
			int32_t					aMin,
			int32_t					aMax)
			: m_min(aMin)
			, m_max(aMax)
		{

		}

		IntRange(
			const SourceNode*		aSource)
		{
			if(aSource->m_type == SourceNode::TYPE_ARRAY)
			{
				if(aSource->m_children.size() == 2)
				{
					m_min = aSource->m_children[0]->GetInt32();
					m_max = aSource->m_children[1]->GetInt32();
				}
				else
				{
					TP_VERIFY(aSource->m_children.size() == 1, aSource->m_debugInfo, "Not a valid range.");
					m_min = aSource->m_children[0]->GetInt32();
					m_max = m_min;
				}
			}
			else
			{
				m_min = aSource->GetInt32();
				m_max = m_min;
			}

			TP_VERIFY(m_min <= m_max, aSource->m_debugInfo, "Minimum larger than maximum.");
		}

		void
		ToStream(
			IWriter*				aWriter) const
		{
			aWriter->WriteInt(m_min);
			aWriter->WriteInt(m_max);
		}

		bool
		FromStream(
			IReader*				aReader)
		{
			if (!aReader->ReadInt(m_min))
				return false;
			if (!aReader->ReadInt(m_max))
				return false;
			if(m_min > m_max)
				return false;
			return true;
		}

		int32_t
		GetRandom(
			std::mt19937&			aRandom) const
		{
			assert(m_min <= m_max);
			UniformDistribution distribution(m_min, m_max);
			return distribution(aRandom);
		}

		// Public data
		int32_t		m_min = 0;
		int32_t		m_max = 0;
	};

}