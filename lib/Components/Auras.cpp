#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>

#include <tpublic/Data/Aura.h>

#include <tpublic/Helpers.h>
#include <tpublic/Manifest.h>

namespace tpublic::Components
{

	bool
	Auras::HasEffect(
		AuraEffect::Id								aId,
		SourceEntityInstance*						aOutSourceEntityInstance) const
	{
		int32_t latestTick = INT32_MIN;

		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			if (entry->HasEffect(aId))
			{
				if(aOutSourceEntityInstance == NULL)
					return true;

				if(entry->m_start > latestTick)
				{
					*aOutSourceEntityInstance = entry->m_sourceEntityInstance;

					latestTick = entry->m_start;
				}
			}
		}

		if(latestTick != INT32_MIN)
			return true;

		return false;
	}

	bool			
	Auras::HasAura(
		uint32_t									aAuraId) const
	{
		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			if(entry->m_auraId == aAuraId)
				return true;
		}

		return false;
	}

	Auras::Entry* 
	Auras::GetAura(
		uint32_t									aAuraId)
	{
		for (std::unique_ptr<Entry>& entry : m_entries)
		{
			if (entry->m_auraId == aAuraId)
				return entry.get();
		}

		return NULL;
	}

	MoveSpeed::Id	
	Auras::GetMoveSpeed() const
	{
		MoveSpeed::Id moveSpeed = MoveSpeed::INVALID_ID;

		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
			{
				MoveSpeed::Id t = effect->GetMoveSpeedModifier();
				if(t != MoveSpeed::INVALID_ID)
				{
					if(moveSpeed == MoveSpeed::INVALID_ID || t < moveSpeed)
						moveSpeed = t;
				}
			}
		}

		if(moveSpeed != MoveSpeed::INVALID_ID)
			return moveSpeed;

		return MoveSpeed::ID_NORMAL;
	}

	float			
	Auras::GetAttackHaste(
		const Manifest*								aManifest) const
	{
		float haste = 0.0f;
		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			const Data::Aura* aura = aManifest->GetById<Data::Aura>(entry->m_auraId);

			if(aura->m_statModifiers)
			{
				const std::optional<Modifier>& modifier = aura->m_statModifiers->m_modifiers[Stat::ID_ATTACK_HASTE];
				if(modifier.has_value())
					haste += modifier->m_add;
			}
		}
		return haste;
	}
	
	float			
	Auras::GetSpellHaste(
		const Manifest*								aManifest) const
	{
		float haste = 0.0f;
		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			const Data::Aura* aura = aManifest->GetById<Data::Aura>(entry->m_auraId);

			if (aura->m_statModifiers)
			{
				const std::optional<Modifier>& modifier = aura->m_statModifiers->m_modifiers[Stat::ID_SPELL_HASTE];
				if (modifier.has_value())
					haste += modifier->m_add;
			}
		}
		return haste;
	}

	int32_t
	Auras::FilterDamageInput(
		const Manifest*								aManifest,
		const EntityInstance*						aSource,
		const EntityInstance*						aTarget,
		DirectEffect::DamageType					aDamageType,
		int32_t										aDamage,
		uint32_t									aAbilityId) const
	{
		int32_t damage = aDamage;

		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
			{
				if (effect->CheckRequirements(aManifest, aSource, aTarget))
					damage = effect->FilterDamageInput(aDamageType, damage, aAbilityId);
			}
		}
						
		return damage;
	}

	int32_t
	Auras::FilterDamageOutput(
		const Manifest*								aManifest,
		const EntityInstance*						aSource,
		const EntityInstance*						aTarget,
		DirectEffect::DamageType					aDamageType,
		int32_t										aDamage,
		uint32_t									aAbilityId) const
	{
		int32_t damage = aDamage;

		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
			{
				if (effect->CheckRequirements(aManifest, aSource, aTarget))
					damage = effect->FilterDamageOutput(aManifest, aSource, aTarget, aDamageType, damage, aAbilityId);
			}
		}
						
		return damage;
	}

	int32_t		
	Auras::FilterHealInput(
		const Manifest*								aManifest,
		const EntityInstance*						aSource,
		const EntityInstance*						aTarget,
		int32_t										aHeal,
		uint32_t									aAbilityId) const
	{
		int32_t heal = aHeal;

		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
			{
				if (effect->CheckRequirements(aManifest, aSource, aTarget))
					heal = effect->FilterHealInput(heal, aAbilityId);
			}
		}

		return heal;
	}


	int32_t		
	Auras::FilterHealOutput(
		const Manifest*								aManifest,
		const EntityInstance*						aSource,
		const EntityInstance*						aTarget,
		int32_t										aHeal,
		uint32_t									aAbilityId) const
	{
		int32_t heal = aHeal;

		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
			{
				if (effect->CheckRequirements(aManifest, aSource, aTarget))
					heal = effect->FilterHealOutput(heal, aAbilityId);
			}
		}

		return heal;
	}

	int32_t		
	Auras::FilterThreat(
		const Manifest*								aManifest,
		const EntityInstance*						aSource,
		const EntityInstance*						aTarget,
		int32_t										aThreat,
		uint32_t									aAbilityId) const
	{
		int32_t threat = aThreat;

		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
			{
				if (effect->CheckRequirements(aManifest, aSource, aTarget))
					threat = effect->FilterThreat(threat, aAbilityId);
			}
		}

		return threat;
	}

	float			
	Auras::GetResourceCostMultiplier() const
	{
		float t = 1.0f;

		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
				t *= effect->GetResourceCostMultiplier();
		}

		return t;
	}

	void		
	Auras::OnCombatEvent(
		const Manifest*								aManifest,
		const EntityInstance*						aSource,
		const EntityInstance*						aTarget,
		tpublic::AuraEffectBase::CombatEventType	aType,
		CombatEvent::Id								aCombatEventId,
		uint32_t									aAbilityId,
		std::mt19937*								aRandom,
		IEventQueue*								aEventQueue) const
	{
		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
			{
				if (effect->CheckRequirements(aManifest, aSource, aTarget))
				{
					effect->OnCombatEvent(aManifest, entry->m_auraId, aType, aCombatEventId, aAbilityId, aSource, aTarget, aRandom, aEventQueue);
				}
			}
		}
	}
	 
	void			
	Auras::OnDamageInput(
		const Manifest*								aManifest,
		const EntityInstance*						aSource,
		const EntityInstance*						aTarget,
		DirectEffect::DamageType					aDamageType,
		int32_t										aDamage,
		CombatEvent::Id								aCombatEventId,
		IEventQueue*								aEventQueue,
		const IWorldView*							aWorldView,
		IResourceChangeQueue*						aResourceChangeQueue) const
	{
		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
			{
				if(effect->CheckRequirements(aManifest, aSource, aTarget))
					effect->OnDamageInput(aSource, aTarget, entry->m_sourceEntityInstance, aDamageType, aDamage, aCombatEventId, aEventQueue, aWorldView, aResourceChangeQueue);
			}
		}
	}

	int32_t			
	Auras::FilterDamageInputOnUpdate(
		DirectEffect::DamageType					aDamageType,
		int32_t										aDamage,
		int32_t&									aOutAbsorbed)
	{
		int32_t damage = aDamage;
		for (std::unique_ptr<Entry>& entry : m_entries)
		{
			for (std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
				damage = effect->FilterDamageInputOnUpdate(aDamageType, damage, entry->m_charges, aOutAbsorbed);
		}

		if(damage != aDamage)
			m_seq++;

		return damage;
	}

	bool			
	Auras::UpdateCastTime(
		const Manifest*								aManifest,
		uint32_t									aAbilityId,
		int32_t&									aCastTime)
	{
		bool modified = false;

		for (std::unique_ptr<Entry>& entry : m_entries)
		{
			for (std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
			{
				if (effect->UpdateCastTime(aManifest, aAbilityId, entry->m_charges, aCastTime))
					modified = true;
			}
		}

		if(modified)
			m_seq++;

		return modified;
	}

	bool		
	Auras::RemoveAura(
		uint32_t									aAuraId,
		uint32_t									aMaxRemove,
		uint32_t									aSourceEntityInstanceId)
	{
		uint32_t removed = 0;

		for(size_t i = 0; i < m_entries.size() && removed < aMaxRemove; i++)
		{
			const std::unique_ptr<Entry>& entry = m_entries[i];

			if(entry->m_auraId == aAuraId && (aSourceEntityInstanceId == 0 || entry->m_sourceEntityInstance.m_entityInstanceId == aSourceEntityInstanceId))
			{
				Helpers::RemoveCyclicFromVector(m_entries, i);
				i--;

				removed++;
			}
		}

		return removed > 0;
	}

	bool			
	Auras::RemoveAuraByGroup(
		const Manifest*								aManifest,
		uint32_t									aAuraGroupId,
		uint32_t									aMaxRemove,
		uint32_t									aSourceEntityInstanceId)
	{
		uint32_t removed = 0;

		for (size_t i = 0; i < m_entries.size() && removed < aMaxRemove; i++)
		{
			const std::unique_ptr<Entry>& entry = m_entries[i];
			const Data::Aura* auraData = aManifest->GetById<tpublic::Data::Aura>(entry->m_auraId);

			if (auraData->m_auraGroupId == aAuraGroupId && (aSourceEntityInstanceId == 0 || entry->m_sourceEntityInstance.m_entityInstanceId == aSourceEntityInstanceId))
			{
				Helpers::RemoveCyclicFromVector(m_entries, i);
				i--;

				removed++;
			}
		}

		return removed > 0;
	}

	bool
	Auras::RemoveAurasByFlags(
		const Manifest*								aManifest,
		uint32_t									aFlags,
		uint32_t									aMaxRemove,
		uint32_t									aSourceEntityInstanceId)
	{
		uint32_t removed = 0;

		for(size_t i = 0; i < m_entries.size() && removed < aMaxRemove; i++)
		{
			const std::unique_ptr<Entry>& entry = m_entries[i];
			const Data::Aura* auraData = aManifest->GetById<tpublic::Data::Aura>(entry->m_auraId);

			if((auraData->m_flags & aFlags) == aFlags && (aSourceEntityInstanceId == 0 || entry->m_sourceEntityInstance.m_entityInstanceId == aSourceEntityInstanceId))
			{
				Helpers::RemoveCyclicFromVector(m_entries, i);
				i--;

				removed++;
			}
		}

		return removed > 0;
	}

	void			
	Auras::Reset()
	{
		m_entries.clear();
		m_seq = 0;
	}

	//------------------------------------------------------------------------

	void				
	Auras::OnLoadedFromPersistence(
		const Manifest*								aManifest) 
	{
		for(size_t i = 0; i < m_entries.size(); i++)
		{
			std::unique_ptr<Entry>& entry = m_entries[i];
			const Data::Aura* auraData = aManifest->TryGetById<tpublic::Data::Aura>(entry->m_auraId);

			if(auraData == NULL)
			{
				m_entries.erase(m_entries.begin() + i);
				i--;
			}
			else
			{
				if (auraData->m_auraEffects.size() == 0)
				{
					entry->m_noEffects = true;
				}
				else
				{
					for (const std::unique_ptr<Data::Aura::AuraEffectEntry>& effect : auraData->m_auraEffects)
						entry->m_effects.push_back(std::unique_ptr<AuraEffectBase>(effect->m_auraEffectBase->Copy()));
				}
			}			
		}

		m_seq++;
	}

}