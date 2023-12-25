#include "../Pcheader.h"

#include <tpublic/DirectEffects/Open.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IResourceChangeQueue.h>

namespace tpublic::DirectEffects
{

	void
	Open::FromSource(
		const SourceNode*			/*aSource*/) 
	{
	}

	void
	Open::ToStream(
		IWriter*					aStream) const 
	{
		ToStreamBase(aStream);
	}

	bool
	Open::FromStream(
		IReader*					aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		return true;
	}

	void
	Open::Resolve(
		int32_t						/*aTick*/,
		std::mt19937&				/*aRandom*/,
		const Manifest*				/*aManifest*/,
		CombatEvent::Id				/*aId*/,
		uint32_t					/*aAbilityId*/,
		const EntityInstance*		aSource,
		EntityInstance*				aTarget,
		IResourceChangeQueue*		aCombatResultQueue,
		IAuraEventQueue*			/*aAuraEventQueue*/,
		IThreatEventQueue*			/*aThreatEventQueue*/) 
	{		
		aCombatResultQueue->AddOpenRequest(aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId());
	}

}