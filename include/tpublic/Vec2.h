#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	struct Vec2
	{
		void
		ToStream(
			IWriter*				aStream) const 
		{
			aStream->WriteInt(m_x);
			aStream->WriteInt(m_y);
		}

		bool
		FromStream(
			IReader*				aStream) 
		{
			if (!aStream->ReadInt(m_x))
				return false;
			if (!aStream->ReadInt(m_y))
				return false;
			return true;
		}

		bool
		operator==(
			const Vec2&				aOther) const
		{
			return aOther.m_x == m_x && aOther.m_y == m_y;
		}

		// Public data
		int32_t		m_x = 0;
		int32_t		m_y = 0;
	};

}
