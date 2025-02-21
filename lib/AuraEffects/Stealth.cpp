#include "../Pcheader.h"

#include <tpublic/AuraEffects/Stealth.h>

#include <tpublic/Components/PlayerPrivate.h>

#include <tpublic/Data/Ability.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Manifest.h>

namespace tpublic::AuraEffects
{

	bool		
	Stealth::OnUpdate(
		const SourceEntityInstance& /*aSourceEntityInstance*/,
		uint32_t					aTargetEntityInstanceId,
		uint32_t					/*aUpdate*/,
		SystemBase::Context*		aContext,
		const Manifest*				/*aManifest*/) 
	{
		const EntityInstance* entity = aContext->m_worldView->WorldViewSingleEntityInstance(aTargetEntityInstanceId);
		if(entity != NULL && entity->IsPlayer())
		{
			const Components::PlayerPrivate* playerPrivate = entity->GetComponent<Components::PlayerPrivate>();
			if(playerPrivate != NULL && playerPrivate->HasUseAbility())
			{
				// FIXME: some abilities shouldn't break stealth
				return false;
			}
		}
		return true;
	}

	void
	Stealth::OnCombatEvent(
		const Manifest*				aManifest,
		uint32_t					aAuraId,
		CombatEventType				aType,
		CombatEvent::Id				aCombatEventId,
		uint32_t					aAbilityId,
		const EntityInstance*		/*aSourceEntityInstance*/,
		const EntityInstance*		aTargetEntityInstance,
		std::mt19937*				/*aRandom*/,
		IEventQueue*				aEventQueue) const
	{
		if(aType == COMBAT_EVENT_TYPE_TARGET && CombatEvent::IsHit(aCombatEventId))
		{
			const Data::Ability* ability = aManifest->GetById<Data::Ability>(aAbilityId);
			if(ability->IsOffensive())
				aEventQueue->EventQueueRemoveAura(aTargetEntityInstance->GetEntityInstanceId(), aAuraId);
		}
	}

}