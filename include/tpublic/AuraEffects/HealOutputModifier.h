#pragma once

#include "../AuraEffectBase.h"
#include "../DirectEffect.h"

namespace tpublic
{

	namespace AuraEffects
	{

		struct HealOutputModifier
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_HEAL_OUTPUT_MODIFIER;
			
			HealOutputModifier()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~HealOutputModifier()
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
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "multiplier")
							m_multiplier = aChild->GetFloat();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteFloat(m_multiplier);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadFloat(m_multiplier))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				HealOutputModifier* t = new HealOutputModifier();
				t->CopyBase(this);

				t->m_multiplier = m_multiplier;

				return t;
			}

			int32_t
			FilterHealOutput(
				int32_t						aHeal) const override
			{
				int32_t heal = (int32_t)((float)aHeal * m_multiplier);
				
				return heal;
			}

			// Public data
			float			m_multiplier = 1.0f;
		};

	}

}