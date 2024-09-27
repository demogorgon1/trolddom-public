#include "../Pcheader.h"

#include <tpublic/AuraEffects/Stun.h>

#include <tpublic/Components/Position.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IWorldView.h>

namespace tpublic::AuraEffects
{

	bool			
	Stun::OnApplication(
		const SourceEntityInstance&	aSourceEntityInstance,
		uint32_t					aTargetEntityInstanceId,
		SystemBase::Context*		aContext,
		const Manifest*				/*aManifest*/) 
	{
		aContext->m_eventQueue->EventQueueInterrupt(aSourceEntityInstance.m_entityInstanceId, aTargetEntityInstanceId, 0, 0);
		return true;
	}

	bool		
	Stun::OnUpdate(
		const SourceEntityInstance& aSourceEntityInstance,
		uint32_t					aTargetEntityInstanceId,
		SystemBase::Context*		aContext,
		const Manifest*				/*aManifest*/) 
	{
		if(m_fear)
		{
			const EntityInstance* sourceEntity = aContext->m_worldView->WorldViewSingleEntityInstance(aSourceEntityInstance.m_entityInstanceId);
			const EntityInstance* targetEntity = aContext->m_worldView->WorldViewSingleEntityInstance(aTargetEntityInstanceId);

			if(sourceEntity != NULL && targetEntity != NULL)
			{
				const Components::Position* sourcePosition = sourceEntity->GetComponent<Components::Position>();
				const Components::Position* targetPosition = targetEntity->GetComponent<Components::Position>();

				Vec2 d = { targetPosition->m_position.m_x - sourcePosition->m_position.m_x, targetPosition->m_position.m_y - sourcePosition->m_position.m_y };
				int32_t distanceSquared = d.m_x * d.m_x + d.m_y * d.m_y;

				IEventQueue::EventQueueMoveRequest moveRequest;
				moveRequest.m_type = IEventQueue::EventQueueMoveRequest::TYPE_SIMPLE;
				moveRequest.m_entityInstanceId = aTargetEntityInstanceId;

				if(distanceSquared < 7 * 7)
				{
					// Run away from source until distance of 7
					if (d.m_x != 0 || d.m_y != 0)
					{
						bool horizontal = std::abs(d.m_x) > std::abs(d.m_y);
						bool vertical = std::abs(d.m_y) > std::abs(d.m_x);

						if (d.m_x < 0 && horizontal)
							moveRequest.AddToPriorityList({ -1, 0 });
						else if (d.m_x > 0 && horizontal)
							moveRequest.AddToPriorityList({ 1, 0 });

						if (d.m_y < 0 && vertical)
							moveRequest.AddToPriorityList({ 0, -1 });
						else if (d.m_y > 0 && vertical)
							moveRequest.AddToPriorityList({ 0, 1 });

						if (d.m_x < 0 && !horizontal)
							moveRequest.AddToPriorityList({ -1, 0 });
						else if (d.m_x > 0 && !horizontal)
							moveRequest.AddToPriorityList({ 1, 0 });

						if (d.m_y < 0 && !vertical)
							moveRequest.AddToPriorityList({ 0, -1 });
						else if (d.m_y > 0 && !vertical)
							moveRequest.AddToPriorityList({ 0, 1 });
					}
				}
				else
				{
					// Just wander
					switch((*(aContext->m_random))() % 4)	
					{
					case 0: moveRequest.AddToPriorityList({ -1, 0 }); break;
					case 1: moveRequest.AddToPriorityList({ 1, 0 }); break;
					case 2: moveRequest.AddToPriorityList({ 0, -1 }); break;
					case 3: moveRequest.AddToPriorityList({ 0, 1 }); break;
					}
				}
				
				aContext->m_eventQueue->EventQueueMove(moveRequest);
			}
		}
		return true;
	}

}