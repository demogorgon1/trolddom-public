#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/VisibleAuras.h>

#include <tpublic/Systems/Combat.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Helpers.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/MapData.h>
#include <tpublic/Requirements.h>
#include <tpublic/Resource.h>

namespace tpublic::Systems
{

	Combat::Combat(
		const SystemData*	aData)
		: SystemBase(aData)
	{
		RequireComponent<Components::Auras>();
		RequireComponent<Components::CombatPrivate>();
		RequireComponent<Components::CombatPublic>();
		RequireComponent<Components::Position>();
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
			Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);
			if(combatPublic->m_castInProgress && combatPublic->m_castInProgress->m_targetEntityInstanceId != 0 && combatPublic->m_castInProgress->m_targetEntityInstanceId != aEntityInstanceId)
			{
				const Components::Position* position = GetComponent<Components::Position>(aComponents);
				const EntityInstance* targetEntityInstance = aContext->m_worldView->WorldViewSingleEntityInstance(combatPublic->m_castInProgress->m_targetEntityInstanceId);
				const Components::Position* targetPosition = targetEntityInstance != NULL ? targetEntityInstance->GetComponent<Components::Position>() : NULL;
				const Data::Ability* ability = GetManifest()->GetById<Data::Ability>(combatPublic->m_castInProgress->m_abilityId);

				if(targetPosition != NULL)
				{
					bool shouldStopCasting = false;

					// Out of range?
					if(!ability->AlwaysInRange())
					{
						int32_t distanceSquared = Helpers::CalculateDistanceSquared(position, targetPosition);
						if (distanceSquared > (int32_t)(ability->m_range * ability->m_range))
							shouldStopCasting = true;
					}

					// Dead?
					if(!shouldStopCasting && targetEntityInstance->GetState() == tpublic::EntityState::ID_DEAD)
						shouldStopCasting = true;

					// No line of sight?
					if(!ability->AlwaysInLineOfSight())
					{
						if (!shouldStopCasting && !aContext->m_worldView->WorldViewLineOfSight(position->m_position, targetPosition->m_position))
							shouldStopCasting = true;
					}

					if(shouldStopCasting)
						aContext->m_eventQueue->EventQueueInterrupt(aEntityInstanceId, aEntityInstanceId, 0, 0);
				}
			}

			Components::Auras* auras = GetComponent<Components::Auras>(aComponents);

			for(size_t i = 0; i < auras->m_entries.size(); i++)
			{
				std::unique_ptr<Components::Auras::Entry>& entry = auras->m_entries[i];

				const Data::Aura* aura = GetManifest()->GetById<Data::Aura>(entry->m_auraId);
				if((aura->m_flags & Data::Aura::FLAG_CANCEL_IN_COMBAT) != 0 && aEntityState == EntityState::ID_IN_COMBAT)
					entry->m_cancel = true;

				if ((aura->m_flags & Data::Aura::FLAG_CANCEL_OUTSIDE_COMBAT) != 0 && aEntityState != EntityState::ID_IN_COMBAT)
					entry->m_cancel = true;

				if((aura->m_flags & Data::Aura::FLAG_PERSIST_IN_DEATH) == 0 && aEntityState == EntityState::ID_DEAD)
					entry->m_cancel = true;

				if ((aura->m_flags & Data::Aura::FLAG_CANCEL_ON_DAMAGE) != 0 && combatPublic->m_damageAccum > 0)
					entry->m_cancel = true;

				if((aura->m_flags & Data::Aura::FLAG_CANCEL_INDOOR) != 0)
				{
					const Components::Position* position = GetComponent<Components::Position>(aComponents);
					if(aContext->m_worldView->WorldViewGetMapData()->IsTileIndoor(position->m_position.m_x, position->m_position.m_y))
						entry->m_cancel = true;
				}

				if (aura->m_flags & Data::Aura::FLAG_SINGLE_TARGET)
				{
					const EntityInstance* sourceEntity = aContext->m_worldView->WorldViewSingleEntityInstance(entry->m_sourceEntityInstance.m_entityInstanceId);
					const Components::CombatPublic* sourceCombatPublic = sourceEntity != NULL ? sourceEntity->GetComponent<Components::CombatPublic>() : NULL;
					if(sourceCombatPublic != NULL && sourceCombatPublic->m_singleTargetAuraEntityInstanceId != aEntityInstanceId)
						entry->m_cancel = true;
				}

				if(aura->m_mustNotHaveWorldAuraId != 0 && aContext->m_worldView->WorldViewHasWorldAura(aura->m_mustNotHaveWorldAuraId))
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
						aContext->m_eventQueue->EventQueueChanneling(entry->m_sourceEntityInstance.m_entityInstanceId, castInProgress);
					}
	
					for(size_t j = 0; j < entry->m_effects.size(); j++)
					{
						std::unique_ptr<AuraEffectBase>& effect = entry->m_effects[j];

						bool cancelEffect = false;

						if((aura->m_flags & Data::Aura::FLAG_NO_SOURCE_NEEDED) == 0 && !entry->m_sourceEntityInstance.IsSet())
							cancelEffect = true;

						if(!cancelEffect && !effect->Update(entry->m_sourceEntityInstance, aEntityInstanceId, aContext, GetManifest()))
							cancelEffect = true;

						if(cancelEffect)
						{
							effect->OnFade(entry->m_sourceEntityInstance, aEntityInstanceId, aContext, GetManifest());

							if(effect->ShouldCancelAuraOnFade())
								entry->m_cancel = true;

							effect.reset();
							Helpers::RemoveCyclicFromVector(entry->m_effects, j);
							j--;
						}						
					}
				}

				if(!entry->m_cancel && aura->m_cancelRequirements.size() > 0)
				{
					const EntityInstance* self = aContext->m_worldView->WorldViewSingleEntityInstance(aEntityInstanceId);
					if(self != NULL)
						entry->m_cancel = Requirements::CheckList(GetManifest(), aura->m_cancelRequirements, self, NULL);
				}

				if((aura->m_flags & Data::Aura::FLAG_CHARGED) != 0 && entry->m_charges == 0)
					entry->m_cancel = true;

				if(aura->m_encounterId != 0 && !aContext->m_worldView->WorldViewIsEncounterActive(aura->m_encounterId))
					entry->m_cancel = true;

				if(entry->m_cancel || (!entry->m_noEffects && entry->m_effects.size() == 0) || (entry->m_end != 0 && aContext->m_tick >= entry->m_end))
				{
					for(std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
						effect->OnFade(entry->m_sourceEntityInstance, aEntityInstanceId, aContext, GetManifest());

					entry.reset();
					Helpers::RemoveCyclicFromVector(auras->m_entries, i);
					i--;

					auras->m_seq++;
					auras->SetPendingPersistenceUpdate(tpublic::ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY);
				}
			}

			{
				Components::CombatPrivate* combatPrivate = GetComponent<Components::CombatPrivate>(aComponents);
				float resourceCostMultiplier = auras->GetResourceCostMultiplier();
				if (resourceCostMultiplier != combatPrivate->m_resourceCostMultiplier)
				{
					combatPrivate->m_resourceCostMultiplier = resourceCostMultiplier;
					combatPrivate->SetDirty();
				}
			}
		}

		return EntityState::CONTINUE;
	}

	void
	Combat::UpdatePublic(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
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
			visibleAuras->m_auraFlags = 0;
			visibleAuras->m_colorEffect.reset();
			visibleAuras->m_colorWeaponGlow.reset();
			visibleAuras->m_mountId = 0;
			
			uint32_t colorEffectR = 0;
			uint32_t colorEffectG = 0;
			uint32_t colorEffectB = 0;
			uint32_t colorEffectA = 0;
			uint32_t colorEffectCount = 0;

			uint32_t colorWeaponGlowR = 0;
			uint32_t colorWeaponGlowG = 0;
			uint32_t colorWeaponGlowB = 0;
			uint32_t colorWeaponGlowA = 0;
			uint32_t colorWeaponGlowCount = 0;

			for (const std::unique_ptr<Components::Auras::Entry>& entry : auras->m_entries)
			{
				const Data::Aura* aura = GetManifest()->GetById<Data::Aura>(entry->m_auraId);

				if(aura->m_colorEffect.has_value())
				{
					colorEffectR += (uint32_t)aura->m_colorEffect->m_r;
					colorEffectG += (uint32_t)aura->m_colorEffect->m_g;
					colorEffectB += (uint32_t)aura->m_colorEffect->m_b;
					colorEffectA += (uint32_t)aura->m_colorEffect->m_a;
					colorEffectCount++;
				}

				if (aura->m_colorWeaponGlow.has_value())
				{
					colorWeaponGlowR += (uint32_t)aura->m_colorWeaponGlow->m_r;
					colorWeaponGlowG += (uint32_t)aura->m_colorWeaponGlow->m_g;
					colorWeaponGlowB += (uint32_t)aura->m_colorWeaponGlow->m_b;
					colorWeaponGlowA += (uint32_t)aura->m_colorWeaponGlow->m_a;
					colorWeaponGlowCount++;
				}

				if(aura->m_type != Data::Aura::TYPE_HIDDEN)
				{
					Components::VisibleAuras::Entry t;
					t.m_auraId = entry->m_auraId;
					t.m_entityInstanceId = entry->m_sourceEntityInstance.m_entityInstanceId;
					t.m_start = entry->m_start;
					t.m_end = entry->m_end;
					visibleAuras->m_entries.push_back(t);
				}

				if(aura->m_mountId != 0)
					visibleAuras->m_mountId = aura->m_mountId;

				if(entry->HasEffect(AuraEffect::ID_STUN))
					visibleAuras->m_auraFlags |= Components::VisibleAuras::AURA_FLAG_STUNNED;

				if (entry->HasEffect(AuraEffect::ID_IMMOBILIZE))
					visibleAuras->m_auraFlags |= Components::VisibleAuras::AURA_FLAG_IMMOBILIZED;

				if(entry->HasEffect(AuraEffect::ID_STEALTH))
					visibleAuras->m_auraFlags |= Components::VisibleAuras::AURA_FLAG_STEALTHED;
			}			

			if(colorEffectCount > 0)
			{
				visibleAuras->m_colorEffect = Image::RGBA(
					(uint8_t)(colorEffectR / colorEffectCount),
					(uint8_t)(colorEffectG / colorEffectCount),
					(uint8_t)(colorEffectB / colorEffectCount),
					(uint8_t)(colorEffectA / colorEffectCount)
				);
			}

			if(colorWeaponGlowCount > 0)
			{
				visibleAuras->m_colorWeaponGlow = Image::RGBA(
					(uint8_t)(colorWeaponGlowR / colorWeaponGlowCount),
					(uint8_t)(colorWeaponGlowG / colorWeaponGlowCount),
					(uint8_t)(colorWeaponGlowB / colorWeaponGlowCount),
					(uint8_t)(colorWeaponGlowA / colorWeaponGlowCount)
				);
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

		combatPublic->m_damageAccum = 0;

		if(aEntityState == EntityState::ID_DEAD && !combatPublic->m_doNotZeroResources)
		{
			if(combatPublic->SetZeroResources())
				combatPublic->SetDirty();
		}
	}

}