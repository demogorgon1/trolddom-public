#include "../Pcheader.h"

#include <tpublic/AuraEffects/Stun.h>

#include <tpublic/IEventQueue.h>

namespace tpublic::AuraEffects
{

	bool			
	Stun::OnApplication(
		uint32_t					aSourceEntityInstanceId,
		uint32_t					aTargetEntityInstanceId,
		SystemBase::Context*		aContext,
		const Manifest*				/*aManifest*/) 
	{
		aContext->m_eventQueue->EventQueueInterrupt(aSourceEntityInstanceId, aTargetEntityInstanceId);
		return true;
	}

}