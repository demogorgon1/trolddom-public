#include "../Pcheader.h"

#include <tpublic/AuraEffects/Stun.h>

#include <tpublic/ISystemEventQueue.h>

namespace tpublic::AuraEffects
{

	bool			
	Stun::OnApplication(
		uint32_t					aSourceEntityInstanceId,
		uint32_t					aTargetEntityInstanceId,
		SystemBase::Context*		aContext,
		const Manifest*				/*aManifest*/) 
	{
		aContext->m_systemEventQueue->AddInterruptEvent(aSourceEntityInstanceId, aTargetEntityInstanceId);
		return true;
	}

}