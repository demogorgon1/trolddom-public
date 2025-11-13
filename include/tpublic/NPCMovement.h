#pragma once

#include "IEventQueue.h"
#include "MapPathData.h"
#include "Vec2.h"

namespace tpublic
{

	class NPCMovement
	{
	public:			
		void	SetDirectOnly(
					bool								aDirectOnly);
		bool	GetMoveRequest(						
					const MapPathData*					aMapPathData,
					const Vec2&							aPosition,
					const Vec2&							aDestination,
					int32_t								aCurrentTick,
					int32_t								aLastMoveTick,
					int32_t								aLastAttackTick,
					std::mt19937&						aRandom,
					IEventQueue::EventQueueMoveRequest&	aOut);
		void	Reset(
					int32_t								aCurrentTick);
		bool	ShouldResetIfLOS(
					int32_t								aCurrentTick) const;

	private:

		bool						m_directOnly = false;
		
		enum Mode
		{
			MODE_DIRECT,
			MODE_COMPLEX
		};		

		Mode						m_mode = MODE_DIRECT;
		std::optional<Vec2>			m_storedDestination;
		const MapPathData::Area*	m_currentArea = NULL;
		uint32_t					m_currentAreaId = UINT32_MAX;
		uint32_t					m_destinationAreaId = UINT32_MAX;
		int32_t						m_modeStartTick = 0;
	};

}