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
				const SourceNode*						aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if(aChild->m_name == "fear")
							m_fear = aChild->GetBool();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*								aStream) const override
			{
				ToStreamBase(aStream);

				aStream->WriteBool(m_fear);
			}

			bool
			FromStream(
				IReader*								aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;

				if(!aStream->ReadBool(m_fear))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				Stun* t = new Stun();
				t->CopyBase(this);
				t->m_fear = m_fear;
				return t;
			}

			bool			OnApplication(
								const SourceEntityInstance&	aSourceEntityInstance,
								uint32_t					aTargetEntityInstanceId,
								SystemBase::Context*		aContext,
								const Manifest*				aManifest) override;
			bool			OnUpdate(
								const SourceEntityInstance&	aSourceEntityInstance,
								uint32_t					aTargetEntityInstanceId,
								SystemBase::Context*		aContext,
								const Manifest*				aManifest) override;

			// Public data
			bool		m_fear = false;
		};

	}

}