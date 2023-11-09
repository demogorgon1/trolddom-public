#pragma once

#include "../AuraEffectBase.h"

namespace tpublic
{

	namespace AuraEffects
	{

		struct Immortality
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_IMMORTALITY;

			Immortality()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~Immortality()
			{

			}

			// AuraEffectBase implementation
			void
			FromSource(
				const SourceNode*		/*aSource*/) override
			{
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				AuraEffectBase* t = new Immortality();
				t->CopyBase(this);
				return t;
			}

			// Public data
		};

	}

}