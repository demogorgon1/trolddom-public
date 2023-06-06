#include "../Pcheader.h"

#include <tpublic/Data/Aura.h>

#include <tpublic/DirectEffects/ApplyAura.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IAuraEventQueue.h>
#include <tpublic/IThreatEventQueue.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	namespace DirectEffects
	{

		void	
		ApplyAura::Resolve(
			int32_t					/*aTick*/,
			std::mt19937&			/*aRandom*/,
			const Manifest*			aManifest,
			CombatEvent::Id			/*aId*/,
			uint32_t				aAbilityId,
			const EntityInstance*	aSource,
			EntityInstance*			aTarget,
			IResourceChangeQueue*	/*aCombatResultQueue*/,
			IAuraEventQueue*		aAuraEventQueue,
			IThreatEventQueue*		aThreatEventQueue)
		{
			if(m_threat != 0)
				aThreatEventQueue->AddThreatEvent(aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId(), m_threat);

			// FIXME: filter certain combat event ids. For example, we could have effects not be applied if blocked or only on criticals.
			const Data::Aura* aura = aManifest->m_auras.GetById(m_auraId);

			std::vector<std::unique_ptr<AuraEffectBase>> effects;
			for(const std::unique_ptr<Data::Aura::AuraEffectEntry>& t : aura->m_auraEffects)
			{
				std::unique_ptr<AuraEffectBase> effect(t->m_auraEffectBase->Copy());
				effects.push_back(std::move(effect));
			}

			aAuraEventQueue->ApplyAura(aAbilityId, m_auraId, aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId(), effects); 
		}

	}

}