#include "../Pcheader.h"

#include <tpublic/DirectEffects/StartQuest.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>

namespace tpublic::DirectEffects
{

	void
	StartQuest::FromSource(
		const SourceNode*			aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if(aChild->m_name == "quest")
					m_questId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_QUEST, aChild->GetIdentifier());
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			}
		});
	}

	void
	StartQuest::ToStream(
		IWriter*					aStream) const 
	{
		ToStreamBase(aStream);

		aStream->WriteUInt(m_questId);

	}

	bool
	StartQuest::FromStream(
		IReader*					aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;

		if(!aStream->ReadUInt(m_questId))
			return false;
		return true;
	}

	void
	StartQuest::Resolve(
		int32_t						/*aTick*/,
		std::mt19937&				/*aRandom*/,
		const Manifest*				/*aManifest*/,
		CombatEvent::Id				/*aId*/,
		uint32_t					/*aAbilityId*/,
		const EntityInstance*		/*aSource*/,
		EntityInstance*				aTarget,
		IResourceChangeQueue*		/*aCombatResultQueue*/,
		IAuraEventQueue*			/*aAuraEventQueue*/,
		IEventQueue*				aEventQueue,
		const IWorldView*			/*aWorldView*/) 
	{		
		aEventQueue->EventQueueStartQuest(aTarget->GetEntityInstanceId(), m_questId);
	}

}