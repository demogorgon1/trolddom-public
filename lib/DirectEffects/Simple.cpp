#include "../Pcheader.h"

#include <tpublic/DirectEffects/Simple.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IResourceChangeQueue.h>

namespace tpublic::DirectEffects
{

	void
	Simple::FromSource(
		const SourceNode*			aSource) 
	{
		TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing simple direct effect annotation.");
		m_id = SimpleDirectEffect::StringToId(aSource->m_annotation->GetIdentifier());
		TP_VERIFY(m_id != SimpleDirectEffect::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid simple direct effect.", aSource->m_annotation->GetIdentifier());
	}

	void
	Simple::ToStream(
		IWriter*					aStream) const 
	{
		ToStreamBase(aStream);
		aStream->WritePOD(m_id);
	}

	bool
	Simple::FromStream(
		IReader*					aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		if(!aStream->ReadPOD(m_id))
			return false;
		return true;
	}

	void
	Simple::Resolve(
		int32_t						aTick,
		std::mt19937&				/*aRandom*/,
		const Manifest*				/*aManifest*/,
		CombatEvent::Id				/*aId*/,
		uint32_t					/*aAbilityId*/,
		const EntityInstance*		aSource,
		EntityInstance*				aTarget,
		IResourceChangeQueue*		aCombatResultQueue,
		IAuraEventQueue*			/*aAuraEventQueue*/,
		IEventQueue*				/*aEventQueue*/,
		const IWorldView*			/*aWorldView*/) 
	{		
		switch(m_id)
		{
		case SimpleDirectEffect::ID_OPEN:
		case SimpleDirectEffect::ID_PUSH:
		case SimpleDirectEffect::ID_MAKE_OFFERING:
		case SimpleDirectEffect::ID_PRAY:
		case SimpleDirectEffect::ID_DESECRATE:
			aCombatResultQueue->AddSimpleDirectEffect(aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId(), m_id);
			break;

			//aCombatResultQueue->AddOpenRequest(aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId());
			//break;

		case SimpleDirectEffect::ID_KILL:
			aCombatResultQueue->AddUpdateCallback([aTarget, aTick]()
			{
				aTarget->SetState(EntityState::ID_DEAD, aTick);
			});
			break;

		//case SimpleDirectEffect::ID_PUSH:
		//	aCombatResultQueue->AddPushRequest(aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId());
		//	break;

		default:
			assert(false);
			break;
		}
		
	}

}