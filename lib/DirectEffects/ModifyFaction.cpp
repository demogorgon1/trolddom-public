#include "../Pcheader.h"

#include <tpublic/Components/CombatPublic.h>

#include <tpublic/DirectEffects/ModifyFaction.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IResourceChangeQueue.h>

namespace tpublic::DirectEffects
{

	void
	ModifyFaction::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if (aChild->m_name == "faction")
					m_factionId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_FACTION, aChild->GetIdentifier());
				else if(aChild->m_name == "target_self")
					m_targetSelf = aChild->GetBool();
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
			}
		});
	}

	void
	ModifyFaction::ToStream(
		IWriter*						aStream) const 
	{	
		ToStreamBase(aStream);

		aStream->WriteInt(m_factionId);
		aStream->WriteBool(m_targetSelf);
	}

	bool
	ModifyFaction::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		
		if(!aStream->ReadUInt(m_factionId))
			return false;
		if(!aStream->ReadBool(m_targetSelf))
			return false;
		return true;
	}

	DirectEffectBase::Result
	ModifyFaction::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					/*aRandom*/,
		const Manifest*					/*aManifest*/,
		CombatEvent::Id					/*aId*/,
		uint32_t						/*aAbilityId*/,
		EntityInstance*					aSource,
		EntityInstance*					aTarget,
		const Vec2&						/*aAOETarget*/,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			aCombatResultQueue,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					/*aEventQueue*/,
		const IWorldView*				/*aWorldView*/) 
	{				
		Components::CombatPublic* combatPublic = m_targetSelf ? 
			aSource->GetComponent<Components::CombatPublic>() : aTarget->GetComponent<Components::CombatPublic>();

		uint32_t factionId = m_factionId;

		aCombatResultQueue->AddUpdateCallback([combatPublic, factionId]()
		{	
			combatPublic->m_factionId = factionId;
			combatPublic->SetDirty();
		});

		return Result();
	}

}