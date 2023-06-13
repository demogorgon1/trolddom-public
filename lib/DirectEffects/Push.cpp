#include "../Pcheader.h"

#include <tpublic/DirectEffects/Push.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IResourceChangeQueue.h>

namespace tpublic::DirectEffects
{

	void
	Push::FromSource(
		const Parser::Node*			/*aSource*/) 
	{
	}

	void
	Push::ToStream(
		IWriter*					aStream) const 
	{
		ToStreamBase(aStream);
	}

	bool
	Push::FromStream(
		IReader*					aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		return true;
	}

	void
	Push::Resolve(
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
		aCombatResultQueue->AddPushRequest(aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId());
	}

}