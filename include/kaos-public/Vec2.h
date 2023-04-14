#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace kaos_public
{

	struct Vec2
	{
		void
		ToStream(
			IWriter*				aStream) const 
		{
			aStream->WriteUInt(m_x);
			aStream->WriteUInt(m_y);
		}

		bool
		FromStream(
			IReader*				aStream) 
		{
			if (!aStream->ReadUInt(m_x))
				return false;
			if (!aStream->ReadUInt(m_y))
				return false;
			return true;
		}

		// Public data
		int32_t		m_x = 0;
		int32_t		m_y = 0;
	};

}
