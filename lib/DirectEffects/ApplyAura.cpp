#include "../Pcheader.h"

#include <tpublic/Data/Aura.h>

#include <tpublic/DirectEffects/ApplyAura.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IAuraEventQueue.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	namespace DirectEffects
	{

		void
		ApplyAura::FromSource(
			const SourceNode*				aSource) 
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if (!FromSourceBase(aChild))
				{
					if (aChild->m_name == "aura")
						m_auraId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_AURA, aChild->GetIdentifier());
					else if (aChild->m_name == "threat")
						m_threat = aChild->GetInt32();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				}
			});
		}

		void
		ApplyAura::ToStream(
			IWriter*						aStream) const 
		{
			ToStreamBase(aStream);
			aStream->WriteUInt(m_auraId);
			aStream->WriteInt(m_threat);
		}

		bool
		ApplyAura::FromStream(
			IReader*						aStream) 
		{
			if (!FromStreamBase(aStream))
				return false;
			if (!aStream->ReadUInt(m_auraId))
				return false;
			if (!aStream->ReadInt(m_threat))
				return false;
			return true;
		}

		void	
		ApplyAura::Resolve(
			int32_t							/*aTick*/,
			std::mt19937&					/*aRandom*/,
			const Manifest*					aManifest,
			CombatEvent::Id					/*aId*/,
			uint32_t						aAbilityId,
			EntityInstance*					aSource,
			EntityInstance*					aTarget,
			const ItemInstanceReference&	/*aItem*/,
			IResourceChangeQueue*			/*aCombatResultQueue*/,
			IAuraEventQueue*				aAuraEventQueue,
			IEventQueue*					aEventQueue,
			const IWorldView*				/*aWorldView*/)
		{
			if(m_threat != 0 && aTarget->GetEntityId() != 0)
				aEventQueue->EventQueueThreat(aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId(), m_threat);

			// FIXME: filter certain combat event ids. For example, we could have effects not be applied if blocked or only on criticals.
			const Data::Aura* aura = aManifest->GetById<tpublic::Data::Aura>(m_auraId);

			std::vector<std::unique_ptr<AuraEffectBase>> effects;
			for(const std::unique_ptr<Data::Aura::AuraEffectEntry>& t : aura->m_auraEffects)
			{
				std::unique_ptr<AuraEffectBase> effect(t->m_auraEffectBase->Copy());
				effects.push_back(std::move(effect));
			}

			aAuraEventQueue->ApplyAura(aAbilityId, m_auraId, aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId(), effects); 
		}

	}

}