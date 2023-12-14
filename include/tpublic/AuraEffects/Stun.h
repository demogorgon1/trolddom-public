#pragma once

#include "../AuraEffectBase.h"

namespace tpublic
{

	namespace AuraEffects
	{

		struct Stun
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_STUN;

			Stun()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~Stun()
			{

			}

			// AuraEffectBase implementation
			void
			FromSource(
				const SourceNode*						/*aSource*/) override
			{
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
				AuraEffectBase* t = new Stun();
				t->CopyBase(this);
				return t;
			}

			bool			OnApplication(
								uint32_t				aSourceEntityInstanceId,
								uint32_t				aTargetEntityInstanceId,
								SystemBase::Context*	aContext,
								const Manifest*			aManifest) override;
		};

	}

}