#pragma once

#include "../AuraEffectBase.h"
#include "../DirectEffect.h"

namespace tpublic
{

	namespace AuraEffects
	{

		struct DamageOutputModifier
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_DAMAGE_OUTPUT_MODIFIER;
			
			DamageOutputModifier()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~DamageOutputModifier()
			{

			}

			// AuraEffectBase implementation
			int32_t			FilterDamageOutput(
								const Manifest*				aManifest,
								const EntityInstance*		aSource,
								const EntityInstance*		aTarget,
								DirectEffect::DamageType	aDamageType,
								int32_t						aDamage) const override;

			void
			FromSource(
				const SourceNode*		aSource) override
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
										m_typeMask |= 1 << (uint32_t)damageType;
									}
								});
						}
						else if (aChild->m_name == "multiplier")
						{
							float f = aChild->GetFloat();
							m_multiplierNumerator = (uint32_t)(f * 1000.0f);
							m_multiplierDenominator = 1000;
						}
						else if (aChild->m_name == "multiplier_num")
						{
							m_multiplierNumerator = aChild->GetInt32();
						}
						else if (aChild->m_name == "multiplier_denom")
						{
							m_multiplierDenominator = aChild->GetInt32();
							TP_VERIFY(m_multiplierDenominator != 0, aChild->m_debugInfo, "Multiplier denominator can't be zero.");
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
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteUInt(m_typeMask);
				aStream->WriteInt(m_multiplierNumerator);
				aStream->WriteInt(m_multiplierDenominator);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadUInt(m_typeMask))
					return false;
				if (!aStream->ReadInt(m_multiplierNumerator))
					return false;
				if (!aStream->ReadInt(m_multiplierDenominator))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				DamageOutputModifier* t = new DamageOutputModifier();
				t->CopyBase(this);

				t->m_typeMask = m_typeMask;
				t->m_multiplierNumerator = m_multiplierNumerator;
				t->m_multiplierDenominator = m_multiplierDenominator;

				return t;
			}

			// Public data
			uint32_t					m_typeMask = 0;
			int32_t						m_multiplierNumerator = 1;
			int32_t						m_multiplierDenominator = 1;
		};

	}

}