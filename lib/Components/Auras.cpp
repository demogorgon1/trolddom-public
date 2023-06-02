#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>

namespace tpublic::Components
{

	bool
	Auras::HasEffect(
		AuraEffect::Id					aId) const
	{
		for(const std::unique_ptr<Entry>& entry : m_entries)
		{
			for (const std::unique_ptr<AuraEffectBase>& effect : entry->m_effects)
			{
				if(effect->m_id == aId)
					return true;
			}
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

	//------------------------------------------------------------------------

	void	
	Auras::ToStream(
		IWriter*						aStream) const 
	{				
		aStream->WriteObjectPointers(m_entries);
	}
			
	bool	
	Auras::FromStream(
		IReader*						aStream) 
	{ 
		if(!aStream->ReadObjectPointers(m_entries))
			return false;
		return true; 
	}

	void				
	Auras::OnLoadedFromPersistence(
		const Manifest*					aManifest) 
	{
		for(std::unique_ptr<Entry>& entry : m_entries)
		{
			const Data::Aura* auraData = aManifest->m_auras.GetById(entry->m_auraId);

			for(const std::unique_ptr<Data::Aura::AuraEffectEntry>& effect : auraData->m_auraEffects)
				entry->m_effects.push_back(std::unique_ptr<AuraEffectBase>(effect->m_auraEffectBase->Copy()));
		}

		m_dirty = true;
	}

}