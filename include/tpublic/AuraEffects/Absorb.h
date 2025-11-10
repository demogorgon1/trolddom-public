#pragma once

#include "../AuraEffectBase.h"

namespace tpublic
{

	namespace AuraEffects
	{

		struct Absorb
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_ABSORB;

			Absorb()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~Absorb()
			{

			}

			// AuraEffectBase implementation
			void
			FromSource(
				const SourceNode*										aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "type_mask")
						{
							aChild->GetArray()->ForEachChild([&](
								const SourceNode* aFlag)
							{
								if (aFlag->IsIdentifier("all"))
								{
									m_typeMask = UINT32_MAX;
								}
								else
								{
									DirectEffect::DamageType damageType = DirectEffect::StringToDamageType(aFlag->GetIdentifier());
									TP_VERIFY(damageType != DirectEffect::INVALID_DAMAGE_TYPE, aFlag->m_debugInfo, "'%s' is not a valid damage type.", aFlag->GetIdentifier());
									m_typeMask |= 1 << (uint32_t)damageType;
								}
							});
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					}
				});
			}

			void
			ToStream(
				IWriter*												aStream) const override
			{
				ToStreamBase(aStream);

				aStream->WritePOD(m_typeMask);
			}

			bool
			FromStream(
				IReader*												aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;

				if(!aStream->ReadPOD(m_typeMask))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				Absorb* t = new Absorb();
				t->CopyBase(this);

				t->m_typeMask = m_typeMask;
				return t;
			}

			int32_t			FilterDamageInputOnUpdate(
								DirectEffect::DamageType				aDamageType,
								int32_t									aDamage,
								uint32_t&								aCharges,
								int32_t&								aOutAbsorbed) override;
		
			// Public data
			uint32_t				m_typeMask = 0;
		};

	}

}