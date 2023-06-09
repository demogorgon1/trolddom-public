#include "../Pcheader.h"

#include <tpublic/DirectEffects/Kill.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IResourceChangeQueue.h>

namespace tpublic::DirectEffects
{

	void
	Kill::FromSource(
		const Parser::Node*			/*aSource*/) 
	{
	}

	void
	Kill::ToStream(
		IWriter*					aStream) const 
	{
		ToStreamBase(aStream);
	}

	bool
	Kill::FromStream(
		IReader*					aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		return true;
	}

	void
	Kill::Resolve(
		int32_t						aTick,
		std::mt19937&				/*aRandom*/,
		const Manifest*				/*aManifest*/,
		CombatEvent::Id				/*aId*/,
		uint32_t					/*aAbilityId*/,
		const EntityInstance*		/*aSource*/,
		EntityInstance*				aTarget,
		IResourceChangeQueue*		aCombatResultQueue,
		IAuraEventQueue*			/*aAuraEventQueue*/,
		IThreatEventQueue*			/*aThreatEventQueue*/) 
	{
		aCombatResultQueue->AddUpdateCallback([aTarget, aTick]()
		{
			aTarget->SetState(EntityState::ID_DEAD, aTick);
		});
	}

}