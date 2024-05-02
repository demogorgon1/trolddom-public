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
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*												aStream) const override
			{
				ToStreamBase(aStream);
			}

			bool
			FromStream(
				IReader*												aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				Absorb* t = new Absorb();
				t->CopyBase(this);
				return t;
			}

			int32_t			FilterDamageInputOnUpdate(
								DirectEffect::DamageType				aDamageType,
								int32_t									aDamage,
								uint32_t&								aCharges,
								int32_t&								aOutAbsorbed) override;
		};

	}

}