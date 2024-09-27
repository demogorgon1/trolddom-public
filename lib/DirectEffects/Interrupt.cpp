#include "../Pcheader.h"

#include <tpublic/Components/CombatPublic.h>

#include <tpublic/Data/Cooldown.h>

#include <tpublic/DirectEffects/Interrupt.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/Manifest.h>

namespace tpublic::DirectEffects
{

	void
	Interrupt::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "lockout_ticks")
				m_lockoutTicks = aChild->GetInt32();
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
		});
	}

	void
	Interrupt::ToStream(
		IWriter*						aStream) const 
	{	
		ToStreamBase(aStream);
		aStream->WriteInt(m_lockoutTicks);
	}

	bool
	Interrupt::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		if(!aStream->ReadInt(m_lockoutTicks))
			return false;
		return true;
	}

	DirectEffectBase::Result
	Interrupt::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					/*aRandom*/,
		const Manifest*					aManifest,
		CombatEvent::Id					/*aId*/,
		uint32_t						/*aAbilityId*/,
		const SourceEntityInstance&		/*aSourceEntityInstance*/,
		EntityInstance*					aSource,
		EntityInstance*					aTarget,
		const Vec2&						/*aAOETarget*/,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			/*aCombatResultQueue*/,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					aEventQueue,
		const IWorldView*				/*aWorldView*/) 
	{				
		const Components::CombatPublic* targetCombatPublic = aTarget->GetComponent<Components::CombatPublic>();
		if(targetCombatPublic->m_castInProgress.has_value())
		{
			const Data::Ability* ability = aManifest->GetById<Data::Ability>(targetCombatPublic->m_castInProgress->m_abilityId);

			if(ability->IsInterruptable())
			{
				for (uint32_t cooldownId : ability->m_cooldowns)
				{
					const Data::Cooldown* cooldown = aManifest->GetById<Data::Cooldown>(cooldownId);
					if (cooldown->m_trigger == Data::Cooldown::TRIGGER_INTERRUPT)
						aEventQueue->EventQueueInterrupt(aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId(), cooldownId, m_lockoutTicks);
				}
			}
		}

		return Result();
	}

}