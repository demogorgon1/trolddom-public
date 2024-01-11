#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>

#include <tpublic/Helpers.h>

namespace tpublic::Components
{

	bool
	Auras::HasEffect(
		AuraEffect::Id					aId) const
	{
		for(const std::unique_ptr<Entry>& entry : m_entries)
		{
			if(entry->HasEffect(aId))
				return true;
		}

		return false;
	}

	int32_t
	Auras::FilterDamageInput(
		DirectEffect::DamageType		aDamageType,
		int32_t							aDamage) const
	{
		int32_t damage = aDamage;

		for (const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
			{
				damage = effect->FilterDamageInput(aDamageType, aDamage);
			}
		}
						
		return damage;
	}

	void		
	Auras::RemoveAura(
		uint32_t						aAuraId)
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
		const Manifest*					aManifest) 
	{
		for(std::unique_ptr<Entry>& entry : m_entries)
		{
			const Data::Aura* auraData = aManifest->GetById<tpublic::Data::Aura>(entry->m_auraId);

			for(const std::unique_ptr<Data::Aura::AuraEffectEntry>& effect : auraData->m_auraEffects)
				entry->m_effects.push_back(std::unique_ptr<AuraEffectBase>(effect->m_auraEffectBase->Copy()));
		}

		m_seq++;
	}

}