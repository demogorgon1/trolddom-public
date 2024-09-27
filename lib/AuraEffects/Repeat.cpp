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
		const SourceEntityInstance&	aSourceEntityInstance,
		uint32_t					aTargetEntityInstanceId,
		SystemBase::Context*		aContext,
		const Manifest*				aManifest) 
	{
		const Data::Ability* ability = aManifest->GetById<tpublic::Data::Ability>(m_abilityId);
		aContext->m_eventQueue->EventQueueAbility(aSourceEntityInstance, aTargetEntityInstanceId, Vec2(), ability, ItemInstanceReference(), NULL);
		return true;
	}

}