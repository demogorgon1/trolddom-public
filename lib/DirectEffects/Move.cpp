#include "../Pcheader.h"

#include <tpublic/Components/Position.h>

#include <tpublic/DirectEffects/Move.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Manifest.h>
#include <tpublic/MapData.h>

namespace tpublic
{

	namespace DirectEffects
	{

		void
		Move::FromSource(
			const SourceNode*				aSource) 
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if (!FromSourceBase(aChild))
				{
					if (aChild->m_name == "destination")
						m_destination = SourceToDestination(aChild);
					else if (aChild->m_name == "move_flags")
						m_moveFlags = SourceToMoveFlags(aChild);
					else if (aChild->m_name == "max_steps")
						m_maxSteps = aChild->GetUInt32();
					else if (aChild->m_name == "min_steps")
						m_minSteps = aChild->GetUInt32();
					else if(aChild->m_name == "trigger_ability_on_resolve")
						m_triggerAbilitiesOnResolve = SecondaryAbility(aChild);
					else if(aChild->m_name == "map_player_spawn")
						m_mapPlayerSpawnId = aChild->GetId(DataType::ID_MAP_PLAYER_SPAWN);
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				}
			});
		}

		void
		Move::ToStream(
			IWriter*						aStream) const 
		{
			ToStreamBase(aStream);
			aStream->WritePOD(m_destination);
			aStream->WritePOD(m_moveFlags);
			aStream->WriteUInt(m_maxSteps);
			aStream->WriteOptionalObject(m_triggerAbilitiesOnResolve);
			aStream->WriteUInt(m_mapPlayerSpawnId);
			aStream->WriteUInt(m_minSteps);
		}

		bool
		Move::FromStream(
			IReader*						aStream) 
		{
			if (!FromStreamBase(aStream))
				return false;
			if (!aStream->ReadPOD(m_destination))
				return false;
			if (!aStream->ReadPOD(m_moveFlags))
				return false;
			if(!aStream->ReadUInt(m_maxSteps))
				return false;
			if(!aStream->ReadOptionalObject(m_triggerAbilitiesOnResolve))
				return false;
			if (!aStream->ReadUInt(m_mapPlayerSpawnId))
				return false;
			if (!aStream->ReadUInt(m_minSteps))
				return false;
			return true;
		}

		DirectEffectBase::Result
		Move::Resolve(
			int32_t							/*aTick*/,
			std::mt19937&					aRandom,
			const Manifest*					/*aManifest*/,
			CombatEvent::Id					/*aId*/,
			uint32_t						/*aAbilityId*/,
			const SourceEntityInstance&		/*aSourceEntityInstance*/,
			EntityInstance*					aSource,
			EntityInstance*					aTarget,
			const Vec2&						aAOETarget,
			const ItemInstanceReference&	/*aItem*/,
			IResourceChangeQueue*			/*aCombatResultQueue*/,
			IAuraEventQueue*				/*aAuraEventQueue*/,
			IEventQueue*					aEventQueue,
			const IWorldView*				aWorldView)
		{
			if(m_moveFlags & MOVE_FLAG_AFFECT_TARGET)
				aSource = aTarget;

			if(aSource != NULL)
			{
				if(m_mapPlayerSpawnId != 0)
				{
					std::optional<Vec2> destinationPosition;

					for(const MapData::PlayerSpawn& playerSpawn : aWorldView->WorldViewGetMapData()->m_playerSpawns)
					{
						if(playerSpawn.m_id == m_mapPlayerSpawnId)
						{	
							destinationPosition = { playerSpawn.m_x, playerSpawn.m_y };
							break;
						}
					}

					if(destinationPosition.has_value())
					{
						const Components::Position* sourcePosition = aSource->GetComponent<Components::Position>();
						Vec2 d = { destinationPosition->m_x - sourcePosition->m_position.m_x, destinationPosition->m_y - sourcePosition->m_position.m_y };

						IEventQueue::EventQueueMoveRequest t;
						t.AddToPriorityList(d);
						t.m_type = IEventQueue::EventQueueMoveRequest::TYPE_SIMPLE;
						t.m_entityInstanceId = aSource->GetEntityInstanceId();
						t.m_setUpdatedOnServerFlag = true;

						if (m_moveFlags & MOVE_FLAG_SET_TELEPORTED)
							t.m_setTeleportedFlag = true;

						aEventQueue->EventQueueMove(t);
					}
				}
				else
				{
					switch(m_destination)
					{
					case DESTINATION_AOE_CENTER:				
						{
							const Components::Position* sourcePosition = aSource->GetComponent<Components::Position>();
							Vec2 d = { aAOETarget.m_x - sourcePosition->m_position.m_x, aAOETarget.m_y - sourcePosition->m_position.m_y };

							IEventQueue::EventQueueMoveRequest t;
							t.AddToPriorityList(d);
							t.m_type = IEventQueue::EventQueueMoveRequest::TYPE_SIMPLE;
							t.m_entityInstanceId = aSource->GetEntityInstanceId();
							t.m_setUpdatedOnServerFlag = true;

							if(m_moveFlags & MOVE_FLAG_SET_TELEPORTED)
								t.m_setTeleportedFlag = true;

							aEventQueue->EventQueueMove(t);

							// Effect doesn't have a target, but we still want a combat log event
							Result result;
							result.m_generateImmediateCombatLogEvent = true;
							return result;
						}
						break;

					case DESTINATION_TARGET_ADJACENT:
						{
							// We can't resolve this here in a good way because target could be moving. Needs to be done sequentially.
							IEventQueue::EventQueueMoveAdjacentRequest t;
							t.m_entityInstanceId = aSource->GetEntityInstanceId();
							t.m_adjacentEntityInstanceId = aTarget->GetEntityInstanceId();
					
							if(m_moveFlags & MOVE_FLAG_WALKABLE_PATH_REQUIRED)
								t.m_maxSteps = m_maxSteps;

							if(m_triggerAbilitiesOnResolve.has_value())
								t.m_triggerAbilityOnResolve = &(m_triggerAbilitiesOnResolve.value());

							aEventQueue->EventQueueMoveAdjacent(t);
						}
						break;

					case DESTINATION_TARGET_NEARBY_RANDOM:	
						{
							const Components::Position* targetPosition = aTarget->GetComponent<Components::Position>();
							const Components::Position* sourcePosition = aSource->GetComponent<Components::Position>();
							
							std::vector<Vec2> nearbyPositions;
							aWorldView->WorldViewGetMapData()->GetWalkableFloodFillPositions(targetPosition->m_position, m_minSteps, m_maxSteps, nearbyPositions);
							if(nearbyPositions.size() > 0)
							{
								const Vec2& position = Helpers::RandomItem(aRandom, nearbyPositions);
								Vec2 d = { position.m_x - sourcePosition->m_position.m_x, position.m_y - sourcePosition->m_position.m_y };

								IEventQueue::EventQueueMoveRequest t;
								t.AddToPriorityList(d);
								t.m_type = IEventQueue::EventQueueMoveRequest::TYPE_SIMPLE;
								t.m_entityInstanceId = aSource->GetEntityInstanceId();
								t.m_setUpdatedOnServerFlag = true;

								if (m_moveFlags & MOVE_FLAG_SET_TELEPORTED)
									t.m_setTeleportedFlag = true;

								aEventQueue->EventQueueMove(t);
							}
						}
						break;

					default:
						break;
					}
				}
			}

			return Result();
		}

	}

}