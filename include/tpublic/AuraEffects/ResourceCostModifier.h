#pragma once

#include "../AuraEffectBase.h"
#include "../DirectEffect.h"

namespace tpublic
{

	namespace AuraEffects
	{

		struct ResourceCostModifier
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_RESOURCE_COST_MODIFIER;
			
			ResourceCostModifier()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~ResourceCostModifier()
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
					if (!FromSourceBase(aChild))
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
				if (!aStream->ReadFloat(m_multiplier))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				ResourceCostModifier* t = new ResourceCostModifier();
				t->CopyBase(this);

				t->m_multiplier = m_multiplier;

				return t;
			}

			float
			GetResourceCostMultiplier() const override
			{
				return m_multiplier;
			}

			// Public data
			float				m_multiplier = 1.0f;
		};

	}

}