#pragma once

#include <tpublic/Data/Ability.h>

#include "../AuraEffectBase.h"
#include "../DirectEffect.h"

namespace tpublic
{

	class Manifest;

	namespace AuraEffects
	{

		struct CastModifier
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_CAST_MODIFIER;
			
			CastModifier()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~CastModifier()
			{

			}

			// AuraEffectBase implementation
			void
			FromSource(
				const SourceNode*		aSource) override
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
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WritePOD(m_abilityFlags);
				aStream->WriteFloat(m_castTimeModifier);
				aStream->WriteBool(m_useCharge);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				if (!aStream->ReadPOD(m_abilityFlags))
					return false;
				if (!aStream->ReadFloat(m_castTimeModifier))
					return false;
				if(!aStream->ReadBool(m_useCharge))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				CastModifier* t = new CastModifier();
				t->CopyBase(this);

				t->m_abilityFlags = m_abilityFlags;
				t->m_castTimeModifier = m_castTimeModifier;
				t->m_useCharge = m_useCharge;

				return t;
			}

			bool				UpdateCastTime(
									const Manifest*					aManifest,
									uint32_t						aAbilityId,
									uint32_t&						aCharges,
									int32_t&						aCastTime) override;

			// Public data
			uint32_t	m_abilityFlags = 0;
			float		m_castTimeModifier = 1.0f;
			bool		m_useCharge = false;
		};

	}

}