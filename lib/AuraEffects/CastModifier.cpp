#include "../Pcheader.h"

#include <tpublic/AuraEffects/CastModifier.h>

#include <tpublic/Data/Ability.h>

#include <tpublic/Manifest.h>

namespace tpublic::AuraEffects
{

	void
	CastModifier::FromSource(
		const SourceNode* aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if (aChild->m_name == "cast_time_multiplier")
				m_castTimeModifier = aChild->GetFloat();
			else if (aChild->m_name == "ability_flags")
				m_abilityFlags = Data::Ability::GetFlags(aChild);
			else if (aChild->m_name == "use_charge")
				m_useCharge = aChild->GetBool();
			else if (aChild->m_name == "abilities")
				aChild->GetIdArray(DataType::ID_ABILITY, m_abilityIds);
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
		});
	}

	void
	CastModifier::ToStream(
		IWriter* aStream) const 
	{
		ToStreamBase(aStream);
		aStream->WritePOD(m_abilityFlags);
		aStream->WriteFloat(m_castTimeModifier);
		aStream->WriteBool(m_useCharge);
		aStream->WriteUInts(m_abilityIds);
	}

	bool
	CastModifier::FromStream(
		IReader* aStream) 
	{
		if (!FromStreamBase(aStream))
			return false;
		if (!aStream->ReadPOD(m_abilityFlags))
			return false;
		if (!aStream->ReadFloat(m_castTimeModifier))
			return false;
		if (!aStream->ReadBool(m_useCharge))
			return false;
		if (!aStream->ReadUInts(m_abilityIds))
			return false;
		return true;
	}

	AuraEffectBase*
	CastModifier::Copy() const 
	{
		CastModifier* t = new CastModifier();
		t->CopyBase(this);

		t->m_abilityFlags = m_abilityFlags;
		t->m_castTimeModifier = m_castTimeModifier;
		t->m_useCharge = m_useCharge;
		t->m_abilityIds = m_abilityIds;

		return t;
	}

	bool				
	CastModifier::UpdateCastTime(
		const Manifest*					aManifest,
		uint32_t						aAbilityId,
		uint32_t&						aCharges,
		int32_t&						aCastTime) 
	{
		if(m_abilityIds.size() > 0)
		{
			bool hasAbility = false;

			for(uint32_t abilityId : m_abilityIds)
			{
				if(abilityId == aAbilityId)
				{
					hasAbility = true;
					break;
				}
			}

			if (!hasAbility)
				return false;
		}

		const Data::Ability* ability = aManifest->GetById<Data::Ability>(aAbilityId);
		if((ability->m_flags & m_abilityFlags) != m_abilityFlags)
			return false;

		if(m_useCharge)
		{
			if(aCharges == 0)
				return false;

			aCharges--;
		}

		aCastTime = (int32_t)((float)aCastTime * m_castTimeModifier);
			
		return true;
	}

}