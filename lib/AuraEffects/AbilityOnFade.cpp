#include "../Pcheader.h"

#include <tpublic/Data/Ability.h>

#include <tpublic/AuraEffects/AbilityOnFade.h>

#include <tpublic/IEventQueue.h>
#include <tpublic/Manifest.h>

namespace tpublic::AuraEffects
{

	void
	AbilityOnFade::OnFade(
		uint32_t						aSourceEntityInstanceId,
		uint32_t						aTargetEntityInstanceId,
		SystemBase::Context*			aContext,
		const Manifest*					aManifest) 
	{
		if(m_abilityId != 0)
		{
			const Data::Ability* ability = aManifest->GetById<Data::Ability>(m_abilityId);
			aContext->m_eventQueue->EventQueueAbility(aSourceEntityInstanceId, aTargetEntityInstanceId, Vec2(), ability, ItemInstanceReference(), NULL);
		}
	}

}