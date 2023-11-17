#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"

namespace tpublic
{

	class UIntRange
	{
	public:
		UIntRange()
		{

		}

		UIntRange(
			const SourceNode*		aSource)
		{
			if(aSource->m_type == SourceNode::TYPE_ARRAY)
			{
				if(aSource->m_children.size() == 2)
				{
					m_min = aSource->m_children[0]->GetUInt32();
					m_max = aSource->m_children[1]->GetUInt32();
				}
				else
				{
					TP_VERIFY(aSource->m_children.size() == 1, aSource->m_debugInfo, "Not a valid range.");
					m_min = aSource->m_children[0]->GetUInt32();
					m_max = m_min;
				}
			}
			else
			{
				m_min = aSource->GetUInt32();
				m_max = m_min;
			}

			TP_VERIFY(m_min <= m_max, aSource->m_debugInfo, "Minimum larger than maximum.");
		}

		void
		ToStream(
			IWriter*				aWriter) const
		{
			aWriter->WriteUInt(m_min);
			aWriter->WriteUInt(m_max);
		}

		bool
		FromStream(
			IReader*				aReader)
		{
			if (!aReader->ReadUInt(m_min))
				return false;
			if (!aReader->ReadUInt(m_max))
				return false;
			if(m_min > m_max)
				return false;
			return true;
		}

		uint32_t
		GetRandom(
			std::mt19937&			aRandom) const
		{
			assert(m_min <= m_max);
			std::uniform_int_distribution distribution(m_min, m_max);
			return distribution(aRandom);
		}

		// Public data
		uint32_t	m_min = 0;
		uint32_t	m_max = 0;
	};

}