#include "../Pcheader.h"

#include <tpublic/Components/Trigger.h>

#include <tpublic/DirectEffects/ActivateTrigger.h>

#include <tpublic/EntityInstance.h>

namespace tpublic::DirectEffects
{

	void
	ActivateTrigger::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(!FromSourceBase(aChild))
			{
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
			}
		});
	}

	void
	ActivateTrigger::ToStream(
		IWriter*						aStream) const 
	{	
		ToStreamBase(aStream);
	}

	bool
	ActivateTrigger::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		return true;
	}

	DirectEffectBase::Result
	ActivateTrigger::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					/*aRandom*/,
		const Manifest*					/*aManifest*/,
		CombatEvent::Id					/*aId*/,
		uint32_t						/*aAbilityId*/,
		EntityInstance*					/*aSource*/,
		EntityInstance*					/*aTarget*/,
		const Vec2&						/*aAOETarget*/,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			/*aCombatResultQueue*/,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					/*aEventQueue*/,
		const IWorldView*				/*aWorldView*/) 
	{						
		return Result();
	}

}