#pragma once

#include "../DirectEffectBase.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct ApplyAura
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_APPLY_AURA;

			ApplyAura()
			{

			}

			virtual 
			~ApplyAura()
			{

			}

			// EffectBase implementation
			void
			FromSource(
				const Parser::Node*		aSource) override
			{
				aSource->ForEachChild([&](
					const Parser::Node*	aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "aura")
							m_auraId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_AURA, aChild->GetIdentifier());
						else if(aChild->m_name == "threat")
							m_threat = aChild->GetInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteUInt(m_auraId);
				aStream->WriteInt(m_threat);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				if (!aStream->ReadUInt(m_auraId))
					return false;
				if (!aStream->ReadInt(m_threat))
					return false;
				return true;
			}

			void	
			Resolve(
				uint32_t				/*aTick*/,	
				std::mt19937&			/*aRandom*/,
				const Manifest*			aManifest,
				CombatEvent::Id			aId,
				uint32_t				aAbilityId,
				const EntityInstance*	aSource,
				EntityInstance*			aTarget,
				IResourceChangeQueue*	/*aCombatResultQueue*/,
				IAuraEventQueue*		aAuraEventQueue,
				IThreatEventQueue*		aThreatEventQueue) override;

			// Public data
			uint32_t		m_auraId = 0;
			int32_t			m_threat = 0;
		};

	}

}