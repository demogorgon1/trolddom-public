#include "../Pcheader.h"

#include <tpublic/AuraEffects/DamageInputEffect.h>

#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/PlayerMinions.h>
#include <tpublic/Components/Position.h>

#include <tpublic/Data/Ability.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Helpers.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IResourceChangeQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	namespace AuraEffects
	{

		namespace
		{
			void
			_ApplyEffect(
				const Components::Position*			aTargetPosition,
				const EntityInstance*				aEntityInstance,
				const DamageInputEffect::EffectType	aEffectType,
				int32_t								aValue,
				IResourceChangeQueue*				aResourceChangeQueue,
				uint32_t							aAbilityId,
				uint32_t							aSourceEntityId,
				uint32_t							aSourceEntityInstanceId,
				int32_t								aRange)
			{
				if(aRange != 0)
				{
					const Components::Position* position = aEntityInstance->GetComponent<Components::Position>();
					if(position != NULL && aTargetPosition != NULL)
					{
						if(Helpers::CalculateDistanceSquared(position, aTargetPosition) > aRange * aRange)
							return;
					}
				}

				switch(aEffectType)
				{
				case DamageInputEffect::EFFECT_TYPE_RESTORE_HEALTH:
					{
						const Components::CombatPublic* combatPublic = aEntityInstance->GetComponent<Components::CombatPublic>();
						size_t index;
						if(combatPublic->GetResourceIndex(Resource::ID_HEALTH, index))
							aResourceChangeQueue->AddResourceChange(CombatEvent::ID_HIT, DirectEffect::INVALID_DAMAGE_TYPE, aAbilityId, aSourceEntityId, aSourceEntityInstanceId, aEntityInstance->GetEntityInstanceId(), index, aValue, 0, false, false);
					}
					break;

				case DamageInputEffect::EFFECT_TYPE_RESTORE_MANA:
					{
						const Components::CombatPublic* combatPublic = aEntityInstance->GetComponent<Components::CombatPublic>();
						size_t index;
						if(combatPublic->GetResourceIndex(Resource::ID_MANA, index))
							aResourceChangeQueue->AddResourceChange(CombatEvent::ID_HIT, DirectEffect::INVALID_DAMAGE_TYPE, aAbilityId, aSourceEntityId, aSourceEntityInstanceId, aEntityInstance->GetEntityInstanceId(), index, aValue, 0, false, false);
					}
					break;

				default:
					break;
				}
			}
		}

		//-----------------------------------------------------------------------------------------

		void				
		DamageInputEffect::OnDamageInput(
			const Manifest*					aManifest,
			const EntityInstance*			aSource,
			const EntityInstance*			aTarget,
			const SourceEntityInstance&		aAuraSource,
			DirectEffect::DamageType		aDamageType,
			int32_t							aDamage,
			CombatEvent::Id					/*aCombatEventId*/,
			IEventQueue*					aEventQueue,
			const IWorldView*				aWorldView,
			IResourceChangeQueue*			aResourceChangeQueue) const
		{
			if(!DirectEffect::CheckDamageTypeMask(aDamageType, m_typeMask))
				return;

			if(m_auraSourceOnly && aAuraSource.m_entityInstanceId != aSource->GetEntityInstanceId())
				return;

			if(m_healthThreshold != 0)
			{
				// Only when this damage takes the target from at least the threshold to below it
				const Components::CombatPublic* combatPublic = aTarget->GetComponent<Components::CombatPublic>();
				const Components::CombatPublic::ResourceEntry* health = combatPublic != NULL ? combatPublic->GetResourceEntry(Resource::ID_HEALTH) : NULL;
				if(health == NULL || health->m_max == 0 || aDamage <= 0)
					return;

				uint64_t threshold = (uint64_t)health->m_max * (uint64_t)m_healthThreshold;
				uint64_t before = (uint64_t)health->m_current * 100;
				uint64_t after = health->m_current > (uint32_t)aDamage ? (uint64_t)(health->m_current - (uint32_t)aDamage) * 100 : 0;
				if(before < threshold || after >= threshold)
					return;
			}

			if(m_abilityId != 0)
			{
				const Data::Ability* ability = aManifest->GetById<Data::Ability>(m_abilityId);
				aEventQueue->EventQueueAbility({ aTarget->GetEntityInstanceId(), aTarget->GetSeq() }, aTarget->GetEntityInstanceId(), Vec2(), ability);
			}

			const Components::Position* targetPosition = aTarget->GetComponent<Components::Position>();

			for(const Effect& effect : m_effects)
			{
				int32_t effectValue = (int32_t)((float)aDamage * effect.m_factor);
				if(effectValue == 0)
					effectValue = 1;

				if(effect.m_targetFlags & TARGET_FLAG_SOURCE)
				{
					_ApplyEffect(targetPosition, aSource, effect.m_type, effectValue, aResourceChangeQueue, m_combatLogAbilityId, aSource->GetEntityId(), aAuraSource.m_entityInstanceId, effect.m_range);
				}
				
				if(effect.m_targetFlags & TARGET_FLAG_SOURCE_COMBAT_GROUP)
				{
					const Components::CombatPublic* combatPublic = aSource->GetComponent<Components::CombatPublic>();
					if(combatPublic != NULL && combatPublic->m_combatGroupId != 0)
					{
						aWorldView->WorldViewGroupEntityInstances(combatPublic->m_combatGroupId, [&](
							const EntityInstance* aEntityInstance)
						{
							if(aEntityInstance != aSource && aEntityInstance->GetState() != EntityState::ID_DEAD)
								_ApplyEffect(targetPosition, aEntityInstance, effect.m_type, effectValue, aResourceChangeQueue, m_combatLogAbilityId, aSource->GetEntityId(), aAuraSource.m_entityInstanceId, effect.m_range);
							return false; // Continue
						});
					}
				}
				
				if(effect.m_targetFlags & TARGET_FLAG_SOURCE_MINIONS)
				{
					const Components::PlayerMinions* playerMinions = aSource->GetComponent<Components::PlayerMinions>();
					if(playerMinions != NULL)
					{
						for(const Components::PlayerMinions::Minion& minion : playerMinions->m_minions)
						{
							if(minion.m_entityInstanceId != 0)
							{
								const EntityInstance* entityInstance = aWorldView->WorldViewSingleEntityInstance(minion.m_entityInstanceId);
								if(entityInstance != NULL && entityInstance->GetState() != EntityState::ID_DEAD)
									_ApplyEffect(targetPosition, entityInstance, effect.m_type, effectValue, aResourceChangeQueue, m_combatLogAbilityId, aSource->GetEntityId(), aAuraSource.m_entityInstanceId, effect.m_range);
							}
						}
					}
				}
			}
		}

	}

}