#pragma once

#include "../AuraEffectBase.h"

namespace tpublic
{

	namespace AuraEffects
	{

		struct AbilityOnFade
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_ABILITY_ON_FADE;

			AbilityOnFade()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~AbilityOnFade()
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
					if(aChild->m_name == "ability")
						m_abilityId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY, aChild->GetIdentifier());
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*								aStream) const override
			{
				ToStreamBase(aStream);

				aStream->WriteUInt(m_abilityId);
			}

			bool
			FromStream(
				IReader*								aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;

				if (!aStream->ReadUInt(m_abilityId))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				AbilityOnFade* t = new AbilityOnFade();
				t->CopyBase(this);
				t->m_abilityId = m_abilityId;
				return t;
			}

			void		OnFade(
							const SourceEntityInstance&	aSourceEntityInstance,
							uint32_t					aTargetEntityInstanceId,
							SystemBase::Context*		aContext,
							const Manifest*				aManifest) override;

			// Public data
			uint32_t			m_abilityId = 0;
		};

	}

}