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

		//if(aDistance == 0)
		//{
		//	/*			
		//	   x			
		//	*/
		//	return dx == 0 && dy == 0;
		//}
		//else if(aDistance == 1)
		//{
		//	/*
		//	   .
		//	  .x.
		//	   .
		//	*/
		//	return std::abs(dx) + std::abs(dy) <= 1;
		//}
		//else if(aDistance == 2)
		//{
		//	/*
		//	   .
		//	  ...
		//	 ..x..
		//	  ...
		//	   .
		//	*/
		//	return std::abs(dx) + std::abs(dy) <= 2;
		//}
		//else if (aDistance == 3)
		//{
		//	/* .
		//	  ...
		//	 .....
		//	...x...
		//	 .....
		//	  ...
		//	   .
		//	*/
		//	return std::abs(dx) + std::abs(dy) <= 3;
		//}
		//else if (aDistance == 4)
		//{
		//	/* .
		//	 +...+
		//	+.....+
		//	.......
		//   ....x....
		//	.......
		//	+.....+
		//	 +...+
		//	   .
		//	*/
		//	return (std::abs(dx) + std::abs(dy) <= 4) ||
		//		(std::abs(dx) == 2 && std::abs(dy) == 3) ||
		//		(std::abs(dx) == 3 && std::abs(dy) == 2);
		//}
		//else if (aDistance == 5)
		//{
		//	/*  +.+
		//	   +...+
		//	  +.....+
		//	 +.......+
		//	+.........+
		//    .....x.....
		//	+.........+
		//	 +.......+
		//	  +.....+
		//	   +...+
		//		+.+
		//	*/
		//	return (std::abs(dx) + std::abs(dy) <= 6) &&
		//		!((dx == 0 && std::abs(dy) == 6) ||	(std::abs(dx) == 6 && dy == 0));
		//}

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

}