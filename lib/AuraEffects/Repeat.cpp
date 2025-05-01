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
		uint32_t					aUpdate,
		SystemBase::Context*		aContext,
		const Manifest*				aManifest) 
	{
		uint32_t abilityId = m_abilitySequence.empty() ? m_abilityId : m_abilitySequence[(size_t)aUpdate % m_abilitySequence.size()];
		
		const Data::Ability* ability = aManifest->GetById<tpublic::Data::Ability>(abilityId);

		aContext->m_eventQueue->EventQueueAbility(aSourceEntityInstance, aTargetEntityInstanceId, Vec2(), ability, ItemInstanceReference(), NULL);
		return true;
	}

}