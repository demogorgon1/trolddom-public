#include "Pcheader.h"

#include <tpublic/NPCMovement.h>

namespace tpublic
{

	bool	
	NPCMovement::GetMoveRequest(
		const MapPathData*				aMapPathData,
		const Vec2&						aPosition,
		const Vec2&						aDestination,
		int32_t							aStuckTicks,
		IMoveRequestQueue::MoveRequest& aOut)
	{
		switch(m_mode)
		{
		case MODE_DIRECT:
			// Move directly towards the destination, no pathfinding, nice and cheap
			if (aStuckTicks > 8)
			{
				// We've been stuck for a bit, gotta try something more complex
				Reset();

				m_mode = MODE_COMPLEX;
			}
			else
			{					
				Vec2 d = aDestination - aPosition;

				if(d.m_x != 0 || d.m_y != 0)
				{
					bool horizontal = std::abs(d.m_x) > std::abs(d.m_y);
					bool vertical = std::abs(d.m_y) > std::abs(d.m_x);

					if (d.m_x < 0 && horizontal)
						aOut.AddToPriorityList({ -1, 0 });
					else if (d.m_x > 0 && horizontal)
						aOut.AddToPriorityList({ 1, 0 });

					if (d.m_y < 0 && vertical)
						aOut.AddToPriorityList({ 0, -1 });
					else if (d.m_y > 0 && vertical)
						aOut.AddToPriorityList({ 0, 1 });

					if (d.m_x < 0 && !horizontal)
						aOut.AddToPriorityList({ -1, 0 });
					else if (d.m_x > 0 && !horizontal)
						aOut.AddToPriorityList({ 1, 0 });

					if (d.m_y < 0 && !vertical)
						aOut.AddToPriorityList({ 0, -1 });
					else if (d.m_y > 0 && !vertical)
						aOut.AddToPriorityList({ 0, 1 });

					aOut.m_moveRequestType = IMoveRequestQueue::MOVE_REQUEST_TYPE_SIMPLE;

					return true;
				}
			}
			break;

		case MODE_COMPLEX:
			// If position and destination are within same area we'll request explicit pathfinding. 
			// If not in the same area, we'll used the precomputed path data.
			{
				uint32_t currentAreaId = aMapPathData->GetAreaIdByPosition(aPosition);
				uint32_t destinationAreaId = UINT32_MAX;

				if(!m_storedDestination.has_value() || m_storedDestination.value() != aDestination)
				{
					m_storedDestination = aDestination;
					
					destinationAreaId = aMapPathData->GetAreaIdByPosition(aDestination);
				}
				else if(m_destinationAreaId != UINT32_MAX)
				{
					destinationAreaId = m_destinationAreaId;
				}

				if(currentAreaId != UINT32_MAX && destinationAreaId != UINT32_MAX)
				{
					if(currentAreaId == destinationAreaId)
					{
						aOut.AddToPriorityList(aDestination - aPosition);
						aOut.m_moveRequestType = IMoveRequestQueue::MOVE_REQUEST_TYPE_FIND_PATH;
						return true;
					}
					else
					{
						if(m_currentAreaId != currentAreaId)
						{
							m_currentAreaId = currentAreaId;
							m_currentArea = aMapPathData->GetArea(currentAreaId);
						}

						if(m_destinationAreaId != destinationAreaId)
						{
							m_destinationAreaId = destinationAreaId;
						}

						uint32_t goToNeighborAreaId = aMapPathData->GetRouteNextAreaId(m_currentAreaId, m_destinationAreaId);

						if(goToNeighborAreaId != UINT32_MAX)
						{
							const PackedDistanceField* distanceFieldToNeighbor = m_currentArea->GetNeighborDistanceField(goToNeighborAreaId);
							assert(distanceFieldToNeighbor != NULL);

							printf("---\n");
							distanceFieldToNeighbor->DebugPrint();

							Vec2 localAreaPosition = aPosition - m_currentArea->m_origin;

							uint32_t center = distanceFieldToNeighbor->Get(localAreaPosition);

							static const Vec2 DIRECTIONS[4] = { { 0, -1 }, { 0, 1 }, { -1, 0 }, { 1, 0 } };

							for(uint32_t i = 0; i < 4; i++)
							{
								uint32_t value = distanceFieldToNeighbor->Get(localAreaPosition + DIRECTIONS[i]);

								if(value < center)
								{
									aOut.AddToPriorityList(DIRECTIONS[i]);
								}
								else if (center == 0 && (value == 0 || value == UINT32_MAX))
								{
									uint32_t areaId = aMapPathData->GetAreaIdByPosition(aPosition + DIRECTIONS[i]);
									
									if(areaId == goToNeighborAreaId)
									{
										aOut.AddToPriorityList(DIRECTIONS[i]);
									}
								}
							}

							if(aOut.m_priorityListLength > 0)
							{
								aOut.m_moveRequestType = IMoveRequestQueue::MOVE_REQUEST_TYPE_SIMPLE;

								return true;
							}
						}
					}					
				}
				else
				{
					Reset();
				}
			}
			break;

		default:
			assert(false);
			break;
		}

		return false;
	}

	void	
	NPCMovement::Reset()
	{
		m_mode = MODE_DIRECT;
		m_storedDestination.reset();
		m_currentArea = NULL;
		m_currentAreaId = UINT32_MAX;
		m_destinationAreaId = UINT32_MAX;
	}

}