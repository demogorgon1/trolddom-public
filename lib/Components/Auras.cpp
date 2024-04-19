#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>

#include <tpublic/Helpers.h>

namespace tpublic::Components
{

	bool
	Auras::HasEffect(
		AuraEffect::Id								aId) const
	{
		for(const std::unique_ptr<Entry>& entry : m_entries)
		{
			if(entry->HasEffect(aId))
				return true;
		}

		return false;
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

	int32_t
	Auras::FilterDamageInput(
		DirectEffect::DamageType					aDamageType,
		int32_t										aDamage) const
	{
		int32_t damage = aDamage;

		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
				damage = effect->FilterDamageInput(aDamageType, damage);
		}
						
		return damage;
	}

	int32_t
	Auras::FilterDamageOutput(
		DirectEffect::DamageType					aDamageType,
		int32_t										aDamage) const
	{
		int32_t damage = aDamage;

		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
				damage = effect->FilterDamageOutput(aDamageType, damage);
		}
						
		return damage;
	}

	int32_t		
	Auras::FilterHealInput(
		int32_t										aHeal) const
	{
		int32_t heal = aHeal;

		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
				heal = effect->FilterHealInput(heal);
		}

		return heal;
	}

	void		
	Auras::OnCombatEvent(
		tpublic::AuraEffectBase::CombatEventType	aType,
		CombatEvent::Id								aCombatEventId,
		AuraEffectBase::SecondaryAbilityCallback	aCallback) const
	{
		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
				effect->OnCombatEvent(aType, aCombatEventId, aCallback);
		}
	}

	void		
	Auras::RemoveAura(
		uint32_t									aAuraId)
	{
		for(size_t i = 0; i < m_entries.size(); i++)
		{
			if(m_entries[i]->m_auraId == aAuraId)
			{
				Helpers::RemoveCyclicFromVector(m_entries, i);
				i--;
			}
		}
	}

	//------------------------------------------------------------------------

	void				
	Auras::OnLoadedFromPersistence(
		const Manifest*								aManifest) 
	{
		for(std::unique_ptr<Entry>& entry : m_entries)
		{
			const Data::Aura* auraData = aManifest->GetById<tpublic::Data::Aura>(entry->m_auraId);

			if(auraData->m_auraEffects.size() == 0)
			{
				entry->m_noEffects = true;
			}				
			else
			{
				for (const std::unique_ptr<Data::Aura::AuraEffectEntry>& effect : auraData->m_auraEffects)
					entry->m_effects.push_back(std::unique_ptr<AuraEffectBase>(effect->m_auraEffectBase->Copy()));
			}
		}

		m_seq++;
	}

}