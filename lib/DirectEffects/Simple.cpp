#include "../Pcheader.h"

#include <tpublic/DirectEffects/Simple.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IResourceChangeQueue.h>
#include <tpublic/SourceEntityInstance.h>

namespace tpublic::DirectEffects
{

	void
	Simple::FromSource(
		const SourceNode*				aSource) 
	{
		TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing simple direct effect annotation.");
		m_id = SimpleDirectEffect::StringToId(aSource->m_annotation->GetIdentifier());
		TP_VERIFY(m_id != SimpleDirectEffect::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid simple direct effect.", aSource->m_annotation->GetIdentifier());
		const SimpleDirectEffect::Info* info = SimpleDirectEffect::GetInfo(m_id);

		switch(aSource->m_type)
		{
		case SourceNode::TYPE_IDENTIFIER:
			if(info->m_isEntityState)
			{
				EntityState::Id entityState = EntityState::StringToId(aSource->GetIdentifier());
				TP_VERIFY(entityState != EntityState::INVALID_ID, aSource->m_debugInfo, "Invalid entity state.");
				m_param = (uint32_t)entityState;
			}
			else
			{
				TP_VERIFY(info->m_paramType != DataType::INVALID_ID, aSource->m_debugInfo, "Invalid value.");
				m_param = aSource->GetId(info->m_paramType);
			}
			break;

		case SourceNode::TYPE_NUMBER:
			m_param = aSource->GetUInt32();
			break;

		default:
			break;
		}
	}

	void
	Simple::ToStream(
		IWriter*						aStream) const 
	{
		ToStreamBase(aStream);
		aStream->WritePOD(m_id);
		aStream->WriteUInt(m_param);
	}

	bool
	Simple::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		if(!aStream->ReadPOD(m_id))
			return false;
		if(!aStream->ReadUInt(m_param))
			return false;
		return true;
	}

	DirectEffectBase::Result
	Simple::Resolve(
		int32_t							aTick,
		std::mt19937&					/*aRandom*/,
		const Manifest*					/*aManifest*/,
		CombatEvent::Id					/*aId*/,
		uint32_t						/*aAbilityId*/,
		const SourceEntityInstance&		aSourceEntityInstance,
		EntityInstance*					aSource,
		EntityInstance*					aTarget,
		const Vec2&						/*aAOETarget*/,
		const ItemInstanceReference&	aItem,
		IResourceChangeQueue*			aCombatResultQueue,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					aEventQueue,
		const IWorldView*				/*aWorldView*/) 
	{		
		switch(m_id)
		{
		case SimpleDirectEffect::ID_OPEN:
		case SimpleDirectEffect::ID_PUSH:
		case SimpleDirectEffect::ID_PASS:
		case SimpleDirectEffect::ID_PRAY:
		case SimpleDirectEffect::ID_DESECRATE:
		case SimpleDirectEffect::ID_START_QUEST:
		case SimpleDirectEffect::ID_RESURRECT:
		case SimpleDirectEffect::ID_EDIT_PLAYER_WORLDS:
		case SimpleDirectEffect::ID_ACTIVATE_TRIGGER:
		case SimpleDirectEffect::ID_RECALL:
			aCombatResultQueue->AddSimpleDirectEffect(aSourceEntityInstance, aTarget->GetEntityInstanceId(), m_id, m_param);
			break;

		case SimpleDirectEffect::ID_KILL:
			aCombatResultQueue->AddUpdateCallback([aTarget, aTick]()
			{
				aTarget->SetState(EntityState::ID_DEAD, aTick);
			});
			break;

		case SimpleDirectEffect::ID_SET_OWN_STATE:
			if(aSource != NULL)
			{
				aCombatResultQueue->AddUpdateCallback([aSource, aTick, entityState = (EntityState::Id)m_param]()
				{
					aSource->SetState(entityState, aTick);
				});
			}
			break;

		case SimpleDirectEffect::ID_MAKE_OFFERING:
			if(aSource != NULL)
				aEventQueue->EventQueueMakeOffering(aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId(), aItem);
			break;

		default:
			assert(false);
			break;
		}
		
		return Result();
	}

}