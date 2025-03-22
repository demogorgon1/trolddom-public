#include "../Pcheader.h"

#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/ControlPoint.h>
#include <tpublic/Components/Position.h>
#include <tpublic/Components/ThreatTarget.h>

#include <tpublic/DirectEffects/Simple.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IResourceChangeQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/MapData.h>
#include <tpublic/SourceEntityInstance.h>

namespace tpublic::DirectEffects
{
	
	namespace
	{

		uint32_t
		_InfoAndSourceToParam(
			const SimpleDirectEffect::Info*	aInfo,
			const SourceNode*				aSource)
		{
			switch (aSource->m_type)
			{
			case SourceNode::TYPE_IDENTIFIER:
				{
					if (aInfo->m_isEntityState)
					{
						EntityState::Id entityState = EntityState::StringToId(aSource->GetIdentifier());
						TP_VERIFY(entityState != EntityState::INVALID_ID, aSource->m_debugInfo, "Invalid entity state.");
						return (uint32_t)entityState;
					}

					TP_VERIFY(aInfo->m_paramType != DataType::INVALID_ID, aSource->m_debugInfo, "Invalid value.");
					return aSource->GetId(aInfo->m_paramType);
				}

			case SourceNode::TYPE_NUMBER:
				return aSource->GetUInt32();

			default:
				break;
			}		

			TP_VERIFY(false, aSource->m_debugInfo, "Invalid parameter.");
			return 0;
		}

	}

	//----------------------------------------------------------------------------------

	void
	Simple::FromSource(
		const SourceNode*				aSource) 
	{
		TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing simple direct effect annotation.");
		m_id = SimpleDirectEffect::StringToId(aSource->m_annotation->GetIdentifier());
		TP_VERIFY(m_id != SimpleDirectEffect::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid simple direct effect.", aSource->m_annotation->GetIdentifier());
		const SimpleDirectEffect::Info* info = SimpleDirectEffect::GetInfo(m_id);

		if(aSource->m_type == SourceNode::TYPE_OBJECT)
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(!FromSourceBase(aChild))
				{
					if(aChild->m_name == "param")
						m_param = _InfoAndSourceToParam(info, aChild);
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				}					
			});
		}
		else
		{
			m_param = _InfoAndSourceToParam(info, aSource);
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
		std::mt19937&					aRandom,
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
		const IWorldView*				aWorldView) 
	{		
		EntityInstance* target = (m_flags & DirectEffect::FLAG_SELF) != 0 ? aSource : aTarget;

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
		case SimpleDirectEffect::ID_LEARN_RIDING:
			aCombatResultQueue->AddSimpleDirectEffect(aSourceEntityInstance, target->GetEntityInstanceId(), m_id, m_param);
			break;

		case SimpleDirectEffect::ID_KILL:
			aCombatResultQueue->AddUpdateCallback([target, aTick]()
			{
				target->SetState(EntityState::ID_DEAD, aTick);
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
				aEventQueue->EventQueueMakeOffering(aSource->GetEntityInstanceId(), target->GetEntityInstanceId(), aItem);
			break;

		case SimpleDirectEffect::ID_CAPTURE:
			if(aSource != NULL && target != NULL && aSource->IsPlayer())
			{
				const Components::CombatPublic* sourceCombatPublic = aSource->GetComponent<Components::CombatPublic>();
				if(sourceCombatPublic != NULL && sourceCombatPublic->m_factionId != 0)
				{
					const MapData::PVP* pvp = aWorldView->WorldViewGetMapData()->m_pvp.get();
					const MapData::PVPFaction* faction = pvp != NULL ? pvp->GetFaction(sourceCombatPublic->m_factionId) : NULL;
					if(faction != NULL && faction->m_capturedControlPointStateId != 0)
					{
						aCombatResultQueue->AddUpdateCallback([target, capturedControlPointStateId = faction->m_capturedControlPointStateId]()
						{
							Components::ControlPoint* targetControlPoint = target->GetComponent<Components::ControlPoint>();
							if(targetControlPoint != NULL)
							{
								targetControlPoint->SetControlPointStateId(capturedControlPointStateId);
								targetControlPoint->SetDirty();
							}
						});
					}
				}
			}
			break;

		case SimpleDirectEffect::ID_THREAT_TABLE_EQUALIZE:
		case SimpleDirectEffect::ID_THREAT_TABLE_SHUFFLE:
			if(target != NULL)
			{
				Components::ThreatTarget* threatTarget = target->GetComponent<Components::ThreatTarget>();
				if(threatTarget != NULL)
				{
					ThreatTable* threatTable = &threatTarget->m_table;
					uint32_t seed = aRandom();

					aCombatResultQueue->AddUpdateCallback([threatTable, id = m_id, seed]()
					{
						// We don't add or remove any entries from the table so this is easy. No need to create a threat event as we don't involve other entities.
						switch(id)
						{
						case SimpleDirectEffect::ID_THREAT_TABLE_EQUALIZE:	threatTable->Equalize(); break;
						case SimpleDirectEffect::ID_THREAT_TABLE_SHUFFLE:	threatTable->Shuffle(seed); break;

						default:
							break;
						}
					});
				}
			}
			break;

		case SimpleDirectEffect::ID_ZONE_TELEPORT:
			if(target != NULL)
			{
				const Components::Position* position = target->GetComponent<Components::Position>();

				const std::vector<Vec2>* zonePositions = aWorldView->WorldViewQueryZonePositions(m_param);
				if(zonePositions != NULL)
				{
					IEventQueue::EventQueueMoveRequest move;
					move.m_type = IEventQueue::EventQueueMoveRequest::TYPE_SIMPLE;
					move.m_entityInstanceId = target->GetEntityInstanceId();
					move.m_setTeleportedFlag = true;
					move.m_setUpdatedOnServerFlag = true;
					move.m_emptySpacePrio = true;
					move.m_priorityListLength = IEventQueue::EventQueueMoveRequest::MAX_PRIORITY_LIST_LENGTH;
					for(uint32_t i = 0; i < IEventQueue::EventQueueMoveRequest::MAX_PRIORITY_LIST_LENGTH; i++)
					{
						Vec2& p = move.m_priorityList[i];
						p = Helpers::RandomItem(aRandom, *zonePositions);
						p.m_x -= position->m_position.m_x;
						p.m_y -= position->m_position.m_y;
					}
					aEventQueue->EventQueueMove(move);
				}
			}
			break;

		default:
			assert(false);
			break;
		}
		
		return Result();
	}

}