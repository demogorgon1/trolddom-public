#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"

namespace tpublic
{
	
	class Modifier
	{
	public:
		Modifier()
		{

		}

		Modifier(
			const Modifier&		aOther,
			float				aMultiplier)
			: m_add(aOther.m_add * aMultiplier)
			, m_addPercent(aOther.m_addPercent * aMultiplier)
		{

		}

		Modifier(
			const SourceNode*	aSource)
		{
			aSource->ForEachChild([&](
				const SourceNode*	aChild)
			{
				if(aChild->m_name == "add")
					m_add = aChild->GetFloat();
				else if(aChild->m_name == "add_percent")
					m_addPercent = aChild->GetFloat();
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid modifier item.", aChild->m_name.c_str());
			});
		}

		void
		Combine(
			const Modifier&		aOther,
			float				aMultiplier)
		{
			m_add += aOther.m_add * aMultiplier;
			m_addPercent += aOther.m_addPercent * aMultiplier;
		}

		float
		Calculate(
			float			aIn) const
		{
			float value = ((aIn + m_add) * (1.0f + m_addPercent / 100.0f));
			if(value < 0.0f)
				return 0.0f;
			return value;
		}

		void
		ToStream(
			IWriter*			aWriter) const
		{
			aWriter->WriteFloat(m_add);
			aWriter->WriteFloat(m_addPercent);
		}

		bool
		FromStream(
			IReader*			aReader) 
		{
			if(!aReader->ReadFloat(m_add))
				return false;
			if(!aReader->ReadFloat(m_addPercent))
				return false;
			return true;
		}

		// Public data
		float		m_add = 0.0f;
		float		m_addPercent = 0.0f;
	};

}