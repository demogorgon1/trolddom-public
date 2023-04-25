#pragma once

namespace kpublic
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
								uint32_t				aEntityInstanceId,
								const Vec2&				aMove) = 0;
	};

}