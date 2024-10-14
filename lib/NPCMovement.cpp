#include "Pcheader.h"

#include <tpublic/NPCMovement.h>

namespace tpublic
{

	void	
	NPCMovement::SetDirectOnly(
		bool								aDirectOnly)
	{
		m_directOnly = aDirectOnly;
	}

	bool	
	NPCMovement::GetMoveRequest(
		const MapPathData*					aMapPathData,
		const Vec2&							aPosition,
		const Vec2&							aDestination,
		int32_t								aCurrentTick,
		int32_t								aLastMoveTick,
		std::mt19937&						aRandom,
		IEventQueue::EventQueueMoveRequest& aOut)
	{
		switch(m_mode)
		{
		case MODE_DIRECT:
			{
				int32_t ticksSinceLastMove = aCurrentTick - aLastMoveTick;
				// Move directly towards the destination, no pathfinding, nice and cheap
				if (ticksSinceLastMove > 8 && !m_directOnly)
				{
					// We've been stuck for a bit, gotta try something more complex
					Reset(aCurrentTick);
					m_mode = MODE_COMPLEX;
				}
				else if(ticksSinceLastMove > 20 && m_directOnly)
				{
					return false;
				}
				else
				{
					Vec2 d = aDestination - aPosition;

					if (d.m_x != 0 || d.m_y != 0)
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

						// If target is in a perfect diagonal direction we randomize whether we go horizontal or vertical
						bool roll = aRandom() & 0x80000000;
						if(roll)
						{
							if (d.m_x < 0 && !horizontal)
								aOut.AddToPriorityList({ -1, 0 });
							else if (d.m_x > 0 && !horizontal)
								aOut.AddToPriorityList({ 1, 0 });

							if (d.m_y < 0 && !vertical)
								aOut.AddToPriorityList({ 0, -1 });
							else if (d.m_y > 0 && !vertical)
								aOut.AddToPriorityList({ 0, 1 });
						}
						else
						{
							if (d.m_y < 0 && !vertical)
								aOut.AddToPriorityList({ 0, -1 });
							else if (d.m_y > 0 && !vertical)
								aOut.AddToPriorityList({ 0, 1 });

							if (d.m_x < 0 && !horizontal)
								aOut.AddToPriorityList({ -1, 0 });
							else if (d.m_x > 0 && !horizontal)
								aOut.AddToPriorityList({ 1, 0 });
						}


						aOut.m_type = IEventQueue::EventQueueMoveRequest::TYPE_SIMPLE;

						return true;
					}
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
						aOut.m_type = IEventQueue::EventQueueMoveRequest::TYPE_FIND_PATH;
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
								aOut.m_type = IEventQueue::EventQueueMoveRequest::TYPE_SIMPLE;

								return true;
							}
						}
					}					
				}
				else
				{
					Reset(aCurrentTick);
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
	NPCMovement::Reset(
		int32_t				aCurrentTick)
	{
		m_mode = MODE_DIRECT;
		m_storedDestination.reset();
		m_currentArea = NULL;
		m_currentAreaId = UINT32_MAX;
		m_destinationAreaId = UINT32_MAX;
		m_modeStartTick = aCurrentTick;
	}

	bool	
	NPCMovement::ShouldResetIfLOS(
		int32_t				aCurrentTick) const
	{
		if(m_modeStartTick != 0 && m_mode == MODE_COMPLEX)
			return aCurrentTick - m_modeStartTick > 10;

		return false;
	}

}