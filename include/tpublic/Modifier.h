#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace tpublic
{
	
	class Modifier
	{
	public:
		Modifier()
		{

		}

		Modifier(
			const Parser::Node*	aSource)
		{
			aSource->ForEachChild([&](
				const Parser::Node*	aChild)
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
			const Modifier&		aOther)
		{
			m_add += aOther.m_add;
			m_addPercent += aOther.m_addPercent;
		}

		uint32_t
		Calculate(
			uint32_t			aIn) const
		{
			float value = (((float)aIn + m_add) * (1.0f + m_addPercent / 100.0f));
			if(value < 0.0f)
				return 0;
			return (uint32_t)value;
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