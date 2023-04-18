#include "Pcheader.h"

#include <kaos-public/Components/Position.h>

#include <kaos-public/Vec2.h>

namespace kaos_public::Helpers
{

	bool		
	IsWithinDistance(
		const Vec2&					aA,
		const Vec2&					aB,
		int32_t						aDistance)
	{
		int32_t dx = aA.m_x - aB.m_x;
		int32_t dy = aA.m_y - aB.m_y;

		return dx * dx + dy * dy <= aDistance * aDistance;
	}

	bool	
	IsWithinDistance(
		const Components::Position* aA,
		const Components::Position* aB,
		int32_t						aDistance)
	{
		if(aA == NULL || aB == NULL)
			return false;

		return IsWithinDistance(aA->m_position, aB->m_position, aDistance);
	}

	float		
	RandomFloat(
		std::mt19937&				aRandom)
	{
		return (float)((aRandom() & 0xFFFF0000) >> 16) / (float)0x0000FFFF;
	}

}