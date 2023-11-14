#pragma once

#include "Hash.h"
#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	struct Vec2
	{
		struct Hasher
		{
			uint32_t
			operator()(
				const Vec2& aValue) const
			{
				static_assert(sizeof(aValue) == sizeof(uint64_t));
				return (uint32_t)(Hash::Splitmix_64(*((const uint64_t*)&aValue)) & 0xFFFFFFFFULL);
			}
		};

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
			const Vec2& aOther) const
		{
			return aOther.m_x == m_x && aOther.m_y == m_y;
		}

		// Public data
		int32_t		m_x = 0;
		int32_t		m_y = 0;
	};

	inline Vec2
	operator +(
		const Vec2&					 aLHS,
		const Vec2&					 aRHS)
	{
		return { aLHS.m_x + aRHS.m_x, aLHS.m_y + aRHS.m_y };
	}

	inline Vec2
	operator -(
		const Vec2&					aLHS,
		const Vec2&					aRHS)
	{
		return { aLHS.m_x - aRHS.m_x, aLHS.m_y - aRHS.m_y };
	}

}
