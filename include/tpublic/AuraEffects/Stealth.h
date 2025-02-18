#pragma once

#include "../AuraEffectBase.h"

namespace tpublic
{

	namespace AuraEffects
	{

		struct Stealth
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_STEALTH;

			Stealth()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~Stealth()
			{

			}

			// AuraEffectBase implementation
			void
			FromSource(
				const SourceNode*						aSource) override
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
				IWriter*								aStream) const override
			{
				ToStreamBase(aStream);
			}

			bool
			FromStream(
				IReader*								aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				Stealth* t = new Stealth();
				t->CopyBase(this);
				return t;
			}
		};

	}

}