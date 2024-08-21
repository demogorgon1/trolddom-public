#include "../Pcheader.h"

#include <tpublic/DirectEffects/LearnProfessionAbility.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>

namespace tpublic::DirectEffects
{

	void
	LearnProfessionAbility::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if (aChild->m_name == "profession")
					m_professionId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_PROFESSION, aChild->GetIdentifier());
				else if (aChild->m_name == "ability")
					m_abilityId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY, aChild->GetIdentifier());
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
			}
		});
	}

	void
	LearnProfessionAbility::ToStream(
		IWriter*						aStream) const 
	{	
		ToStreamBase(aStream);

		aStream->WriteUInt(m_professionId);
		aStream->WriteUInt(m_abilityId);
	}

	bool
	LearnProfessionAbility::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;

		if (!aStream->ReadUInt(m_professionId))
			return false;
		if (!aStream->ReadUInt(m_abilityId))
			return false;
		return true;
	}

	DirectEffectBase::Result
	LearnProfessionAbility::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					/*aRandom*/,
		const Manifest*					/*aManifest*/,
		CombatEvent::Id					/*aId*/,
		uint32_t						/*aAbilityId*/,
		EntityInstance*					/*aSource*/,
		EntityInstance*					aTarget,
		const Vec2&						/*aAOETarget*/,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			/*aCombatResultQueue*/,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					aEventQueue,
		const IWorldView*				/*aWorldView*/) 
	{				
		aEventQueue->EventQueueLearnProfessionAbility(aTarget->GetEntityInstanceId(), m_professionId, m_abilityId);
		return Result();
	}

}