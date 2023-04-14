#pragma once

namespace kaos_public
{

	namespace Components
	{
		struct Position;
	}

	struct Vec2;

	namespace Helpers
	{

		bool		IsWithinDistance(
						const Vec2&					aA,
						const Vec2&					aB,
						int32_t						aDistance);
		bool		IsWithinDistance(
						const Components::Position*	aA,
						const Components::Position*	aB,
						int32_t						aDistance);
		
	}

}