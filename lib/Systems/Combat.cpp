#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/VisibleAuras.h>

#include <tpublic/Systems/Combat.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/Resource.h>
#include <tpublic/Helpers.h>

namespace tpublic::Systems
{

	Combat::Combat(
		const SystemData*	aData)
		: SystemBase(aData)
	{
		RequireComponent<Components::Auras>();
		RequireComponent<Components::CombatPublic>();
		RequireComponent<Components::VisibleAuras>();
	}
	
	Combat::~Combat()
	{

	}

	//---------------------------------------------------------------------------

	EntityState::Id
	Combat::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			aEntityInstanceId,
		EntityState::Id		aEntityState,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			aContext) 
	{
		{
			Components::Auras* auras = GetComponent<Components::Auras>(aComponents);

			for(size_t i = 0; i < auras->m_entries.size(); i++)
			{
				std::unique_ptr<Components::Auras::Entry>& entry = auras->m_entries[i];

				const Data::Aura* aura = GetManifest()->GetById<Data::Aura>(entry->m_auraId);
				if((aura->m_flags & Data::Aura::FLAG_CANCEL_IN_COMBAT) != 0 && aEntityState == EntityState::ID_IN_COMBAT)
					entry->m_cancel = true;

				if((aura->m_flags & Data::Aura::FLAG_PERSIST_IN_DEATH) == 0 && aEntityState == EntityState::ID_DEAD)
					entry->m_cancel = true;
				
				if(!entry->m_cancel)
				{
					if(entry->m_channeledAbilityId != 0)
					{
						CastInProgress castInProgress;
						castInProgress.m_abilityId = entry->m_channeledAbilityId;
						castInProgress.m_channeling = true;
						castInProgress.m_start = entry->m_start;
						castInProgress.m_end = entry->m_end;
						castInProgress.m_targetEntityInstanceId = aEntityInstanceId;
						aContext->m_eventQueue->EventQueueChanneling(entry->m_entityInstanceId, castInProgress);
					}
	
					for(size_t j = 0; j < entry->m_effects.size(); j++)
					{
						std::unique_ptr<AuraEffectBase>& effect = entry->m_effects[j];

						if(!effect->Update(entry->m_entityInstanceId, aEntityInstanceId, aContext, GetManifest()))
						{
							effect.reset();
							Helpers::RemoveCyclicFromVector(entry->m_effects, j);
							j--;
						}						
					}
				}

				if(entry->m_cancel || (!entry->m_noEffects && entry->m_effects.size() == 0) || (entry->m_end != 0 && aContext->m_tick >= entry->m_end))
				{
					entry.reset();
					Helpers::RemoveCyclicFromVector(auras->m_entries, i);
					i--;

					auras->m_seq++;
					auras->SetPendingPersistenceUpdate(tpublic::ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY);
				}
			}
		}

		return EntityState::CONTINUE;
	}

	void
	Combat::UpdatePublic(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			/*aContext*/) 
	{
		Components::Auras* auras = GetComponent<Components::Auras>(aComponents);
		Components::VisibleAuras* visibleAuras = GetComponent<Components::VisibleAuras>(aComponents);

		if(auras->m_seq > visibleAuras->m_seq)
		{
			visibleAuras->m_seq = auras->m_seq;
			visibleAuras->m_entries.clear();
			visibleAuras->m_stunned = false;

			for (const std::unique_ptr<Components::Auras::Entry>& entry : auras->m_entries)
			{
				const Data::Aura* aura = GetManifest()->GetById<Data::Aura>(entry->m_auraId);

				if(aura->m_type != Data::Aura::TYPE_HIDDEN)
				{
					Components::VisibleAuras::Entry t;
					t.m_auraId = entry->m_auraId;
					t.m_entityInstanceId = entry->m_entityInstanceId;
					t.m_start = entry->m_start;
					t.m_end = entry->m_end;
					visibleAuras->m_entries.push_back(t);
				}

				if(!visibleAuras->m_stunned && entry->HasEffect(AuraEffect::ID_STUN))
					visibleAuras->m_stunned = true;
			}

			visibleAuras->SetDirty();
		}

		MoveSpeed::Id moveSpeed = auras->GetMoveSpeed();		
		Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);
		if(combatPublic->m_moveSpeed != moveSpeed)
		{
			combatPublic->m_moveSpeed = moveSpeed;
			combatPublic->SetDirty();
		}
	}

}