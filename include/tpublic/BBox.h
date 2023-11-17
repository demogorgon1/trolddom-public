#pragma once

#include "Vec2.h"

namespace tpublic
{

	class BBox
	{
	public:
		BBox
		Combine(
			const BBox&		aOther) const
		{
			BBox combined;
			combined.m_min = { Base::Min(m_min.m_x, aOther.m_min.m_x), Base::Min(m_min.m_y, aOther.m_min.m_y) };
			combined.m_max = { Base::Max(m_max.m_x, aOther.m_max.m_x), Base::Max(m_max.m_y, aOther.m_max.m_y) };
			return combined;
		}

		Vec2
		GetSize() const
		{
			return m_max - m_min;
		}

		bool
		CheckOverlap(
			const BBox&		aOther) const
		{
			Vec2 size = GetSize();
			Vec2 otherSize = aOther.GetSize();
			Vec2 combinedSize = Combine(aOther).GetSize();
			return combinedSize.m_x < size.m_x + otherSize.m_x && combinedSize.m_y < size.m_y + otherSize.m_y;
		}
	
		// Public data
		Vec2		m_min; 
		Vec2		m_max;
	};

}