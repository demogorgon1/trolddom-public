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
				return aValue.GetHash32();
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

		int32_t
		DistanceSquared(
			const Vec2&				aOther) const
		{
			Vec2 t = { m_x - aOther.m_x, m_y - aOther.m_y };
			return t.m_x * t.m_x + t.m_y * t.m_y;
		}

		uint32_t
		GetHash32() const
		{
			static_assert(sizeof(*this) == sizeof(uint64_t));
			return (uint32_t)(Hash::Splitmix_64(*((const uint64_t*)this)) & 0xFFFFFFFFULL);
		}		

		bool
		operator==(
			const Vec2& aOther) const
		{
			return aOther.m_x == m_x && aOther.m_y == m_y;
		}

		bool
		operator<(
			const Vec2&	aOther) const
		{
			if(aOther.m_x == m_x)
				return m_y < aOther.m_y;
			return m_x < aOther.m_x;
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
