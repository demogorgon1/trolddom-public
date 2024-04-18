#include "../Pcheader.h"

#include <tpublic/AuraEffects/Repeat.h>

#include <tpublic/Data/Ability.h>

#include <tpublic/IEventQueue.h>
#include <tpublic/Manifest.h>
#include <tpublic/Vec2.h>

namespace tpublic::AuraEffects
{

	bool
	Repeat::OnUpdate(
		uint32_t				aSourceEntityInstanceId,
		uint32_t				aTargetEntityInstanceId,
		SystemBase::Context*	aContext,
		const Manifest*			aManifest) 
	{
		const Data::Ability* ability = aManifest->GetById<tpublic::Data::Ability>(m_abilityId);
		aContext->m_eventQueue->EventQueueAbility(aSourceEntityInstanceId, aTargetEntityInstanceId, Vec2(), ability, ItemInstanceReference(), NULL);
		return true;
	}

}