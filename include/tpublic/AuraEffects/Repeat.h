#pragma once

#include "../AuraEffectBase.h"

namespace tpublic
{

	class Manifest;

	namespace AuraEffects
	{

		struct Repeat
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_REPEAT;

			Repeat()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~Repeat()
			{

			}

			// AuraEffectBase implementation
			void
			FromSource(
				const Parser::Node*		aSource) override
			{
				aSource->ForEachChild([&](
					const Parser::Node*	aChild)
				{
					if (!FromSourceBase(aChild))
					{
						if (aChild->m_name == "ability")
							m_abilityId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY, aChild->GetIdentifier());
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

				aStream->WriteUInt(m_abilityId);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;

				if(!aStream->ReadUInt(m_abilityId))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				Repeat* t = new Repeat();
				t->CopyBase(this);

				t->m_abilityId = m_abilityId;

				return t;
			}

			bool					OnUpdate(
										uint32_t				aSourceEntityInstanceId,
										uint32_t				aTargetEntityInstanceId,
										SystemBase::Context*	aContext,
										const Manifest*			aManifest) override;

			// Public data
			uint32_t		m_abilityId = 0;
		};

	}

}