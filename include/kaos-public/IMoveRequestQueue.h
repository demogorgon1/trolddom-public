#pragma once

namespace kaos_public
{

	namespace Components
	{
		struct Position;
	}

	struct Vec2;

	class IMoveRequestQueue
	{
	public:
		virtual				~IMoveRequestQueue() {}

		// Virtual interface
		virtual void		AddMoveRequest(
								Components::Position*	aPosition,
								const Vec2&				aMove) = 0;
	};

}