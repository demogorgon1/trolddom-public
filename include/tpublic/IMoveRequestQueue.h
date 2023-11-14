#pragma once

#include "Vec2.h"

namespace tpublic
{

	namespace Components
	{
		struct Position;
	}

	class IMoveRequestQueue
	{
	public:
		enum MoveRequestType
		{
			MOVE_REQUEST_TYPE_SIMPLE,
			MOVE_REQUEST_TYPE_FIND_PATH
		};

		struct MoveRequest
		{
			static const uint32_t MAX_PRIORITY_LIST_LENGTH = 4;

			void
			AddToPriorityList(
				const Vec2&								aDirection)
			{
				assert(m_priorityListLength < MAX_PRIORITY_LIST_LENGTH);
				m_priorityList[m_priorityListLength++] = aDirection;
			}
			
			// Public data
			MoveRequestType		m_moveRequestType = MoveRequestType(0);
			uint32_t			m_entityInstanceId = 0;
			Vec2				m_priorityList[MAX_PRIORITY_LIST_LENGTH];
			uint32_t			m_priorityListLength = 0;
		};

		virtual				~IMoveRequestQueue() {}

		// Virtual interface
		virtual void		AddMoveRequest(
								const MoveRequest&		aMoveRequest) = 0;
	};

}