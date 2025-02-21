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
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode*	aChild)
				{
					if (!FromSourceBase(aChild))
					{
						if (aChild->m_name == "ability")
							m_abilityId = aChild->GetId(DataType::ID_ABILITY);
						else if(aChild->m_name == "ability_sequence")
							aChild->GetIdArray(DataType::ID_ABILITY, m_abilitySequence);
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
				aStream->WriteUInts(m_abilitySequence);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;

				if(!aStream->ReadUInt(m_abilityId))
					return false;
				if(!aStream->ReadUInts(m_abilitySequence))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				Repeat* t = new Repeat();
				t->CopyBase(this);

				t->m_abilityId = m_abilityId;
				t->m_abilitySequence = m_abilitySequence;

				return t;
			}

			bool					OnUpdate(
										const SourceEntityInstance&	aSourceEntityInstance,
										uint32_t					aTargetEntityInstanceId,
										uint32_t					aUpdate,
										SystemBase::Context*		aContext,
										const Manifest*				aManifest) override;

			// Public data
			uint32_t				m_abilityId = 0;
			std::vector<uint32_t>	m_abilitySequence;
		};

	}

}