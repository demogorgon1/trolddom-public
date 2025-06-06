#pragma once

#include "../Data/NPCBehaviorState.h"

#include "../CastInProgress.h"
#include "../Chat.h"
#include "../Component.h"
#include "../Cooldowns.h"
#include "../DialogueScript.h"
#include "../EntityState.h"
#include "../NPCMovement.h"
#include "../Requirement.h"
#include "../Resource.h"
#include "../ThreatTable.h"

namespace tpublic
{

	namespace Components
	{

		struct NPC
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_NPC;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_NONE;

			struct AggroRequirements
			{
				void
				FromSource(
					const SourceNode*		aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if (aChild->m_tag == "requirement")
							m_requirements.push_back(Requirement(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*				aStream) const
				{
					aStream->WriteObjects(m_requirements);
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if(!aStream->ReadObjects(m_requirements))
						return false;
					return true;
				}	

				// Public data
				std::vector<Requirement>			m_requirements;
			};

			struct OutOfZoneAction
			{
				void
				FromSource(
					const SourceNode*		aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if (aChild->m_name == "use")
							m_useAbilityId = aChild->GetId(DataType::ID_ABILITY);
						else if (aChild->m_name == "evade")
							m_evade = aChild->GetBool();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*				aStream) const
				{
					aStream->WriteUInt(m_useAbilityId);
					aStream->WriteBool(m_evade);
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if(!aStream->ReadUInt(m_useAbilityId))
						return false;
					if (!aStream->ReadBool(m_evade))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_useAbilityId = 0;
				bool					m_evade = false;
			};

			struct AbilityEntry
			{
				enum TargetType : uint8_t
				{
					TARGET_TYPE_DEFAULT,
					TARGET_TYPE_RANDOM_PLAYER,
					TARGET_TYPE_LOW_HEALTH_FRIEND_OR_SELF,
					TARGET_TYPE_SELF
				};

				AbilityEntry()
				{

				}

				AbilityEntry(
					const SourceNode*		aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "id")
							m_abilityId = aChild->GetId(DataType::ID_ABILITY);
						else if(aChild->m_name == "use_probability")
							m_useProbability = aChild->GetProbability();
						else if (aChild->m_name == "target" && aChild->m_type == SourceNode::TYPE_ARRAY)
							aChild->GetIdArray(DataType::ID_ENTITY, m_targetEntityIds);
						else if(aChild->m_name == "target" && aChild->IsIdentifier("random_player"))
							m_targetType = TARGET_TYPE_RANDOM_PLAYER;
						else if (aChild->m_name == "target" && aChild->IsIdentifier("self"))
							m_targetType = TARGET_TYPE_SELF;
						else if (aChild->m_name == "target" && aChild->IsIdentifier("low_health_friend_or_self"))
							m_targetType = TARGET_TYPE_LOW_HEALTH_FRIEND_OR_SELF;
						else if(aChild->m_tag == "requirement")
							m_requirements.push_back(Requirement(aChild));
						else if(aChild->m_name == "update_target")
							m_updateTarget = aChild->GetBool();
						else if (aChild->m_name == "barks")
							aChild->GetArray()->ForEachChild([&](const SourceNode* aBark) { m_barks.push_back(Chat(aBark)); });
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*				aStream) const
				{
					aStream->WriteUInt(m_abilityId);
					aStream->WriteUInt(m_useProbability);
					aStream->WriteUInts(m_targetEntityIds);
					aStream->WritePOD(m_targetType);
					aStream->WriteObjects(m_requirements);
					aStream->WriteBool(m_updateTarget);
					aStream->WriteObjects(m_barks);
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if(!aStream->ReadUInt(m_abilityId))
						return false;
					if (!aStream->ReadUInt(m_useProbability))
						return false;
					if (!aStream->ReadUInts(m_targetEntityIds))
						return false;
					if(!aStream->ReadPOD(m_targetType))
						return false;
					if(!aStream->ReadObjects(m_requirements))
						return false;
					if(!aStream->ReadBool(m_updateTarget))
						return false;
					if (!aStream->ReadObjects(m_barks))
						return false;
					return true;
				}	

				// Public data
				uint32_t							m_abilityId = 0;
				uint32_t							m_useProbability = UINT32_MAX;
				std::vector<uint32_t>				m_targetEntityIds;
				TargetType							m_targetType = TARGET_TYPE_DEFAULT;
				std::vector<Requirement>			m_requirements;
				bool								m_updateTarget = true;				
				std::vector<Chat>					m_barks;
			};

			struct StateEntry
			{
				void
				FromSource(
					const SourceNode*		aSource)
				{
					m_entityState = EntityState::StringToId(aSource->m_name.c_str());
					TP_VERIFY(m_entityState != EntityState::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid entity state.", aSource->GetIdentifier());

					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "abilities")
						{
							aChild->GetArray()->ForEachChild([&](
								const SourceNode* aAbility)
							{
								m_abilities.push_back(AbilityEntry(aAbility));
							});
						}
						else if (aChild->m_name == "barks")
						{
							aChild->GetArray()->ForEachChild([&](
								const SourceNode* aAbility)
							{
								m_barks.push_back(Chat(aAbility));
							});
						}
						else if(aChild->m_name == "dialogue_scripts")
						{
							aChild->GetIdArrayWithLookup<DialogueScript::Id, DialogueScript::INVALID_ID>(m_dialogueScripts, [](
								const char* aIdentifier) -> DialogueScript::Id
							{
								return DialogueScript::StringToId(aIdentifier);
							});
						}
						else if(aChild->m_name == "trigger_ability")
						{
							m_triggerAbilityId = aChild->GetId(DataType::ID_ABILITY);
						}
						else if (aChild->m_name == "on_enter_ability")
						{
							m_onEnterAbilityId = aChild->GetId(DataType::ID_ABILITY);
						}
						else if(aChild->m_name == "despawn_on_leave")
						{
							m_despawnOnLeave = aChild->GetBool();
						}
						else if (aChild->m_name == "despawn_after_ticks")
						{
							m_despawnAfterTicks = aChild->GetUInt32();
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
						}
					});
				}

				void
				ToStream(
					IWriter*				aStream) const
				{
					aStream->WritePOD(m_entityState);
					aStream->WriteObjects(m_abilities);
					aStream->WritePODs(m_dialogueScripts);
					aStream->WriteObjects(m_barks);
					aStream->WriteUInt(m_triggerAbilityId);
					aStream->WriteUInt(m_onEnterAbilityId);
					aStream->WriteBool(m_despawnOnLeave);
					aStream->WriteUInt(m_despawnAfterTicks);
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if(!aStream->ReadPOD(m_entityState))
						return false;
					if(!aStream->ReadObjects(m_abilities))
						return false;
					if(!aStream->ReadPODs(m_dialogueScripts))
						return false;
					if (!aStream->ReadObjects(m_barks))
						return false;
					if(!aStream->ReadUInt(m_triggerAbilityId))
						return false;
					if (!aStream->ReadUInt(m_onEnterAbilityId))
						return false;
					if (!aStream->ReadBool(m_despawnOnLeave))
						return false;
					if (!aStream->ReadUInt(m_despawnAfterTicks))
						return false;
					return true;
				}

				bool
				HasDialogueScript(
					DialogueScript::Id		aDialogueScript) const
				{
					for(DialogueScript::Id t : m_dialogueScripts)
					{
						if(t == aDialogueScript)
							return true;
					}
					return false;
				}

				// Public data
				EntityState::Id						m_entityState = EntityState::ID_DEFAULT;
				std::vector<AbilityEntry>			m_abilities;
				std::vector<DialogueScript::Id>		m_dialogueScripts;
				std::vector<Chat>					m_barks;
				uint32_t							m_triggerAbilityId = 0;
				uint32_t							m_onEnterAbilityId = 0;
				bool								m_despawnOnLeave = false;
				uint32_t							m_despawnAfterTicks = 0;
			};

			struct ResourceEntry
			{
				void
				ToStream(
					IWriter*				aStream) const
				{
					aStream->WriteUInt(m_id);
					aStream->WriteUInt(m_max);
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if (!aStream->ReadUInt(m_id))
						return false;
					if (!aStream->ReadUInt(m_max))
						return false;
					return true;
				}

				// Public data
				uint32_t							m_id = 0;
				uint32_t							m_max = 0;
			};

			struct Resources
			{				
				void
				FromSource(
					const SourceNode*		aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						ResourceEntry t;
						t.m_id = Resource::StringToId(aChild->m_name.c_str());
						TP_VERIFY(t.m_id != 0, aChild->m_debugInfo, "'%s' is not a valid resource.", aChild->m_name.c_str());
						t.m_max = aChild->GetUInt32();
						m_entries.push_back(t);
					});
				}

				void
				ToStream(
					IWriter*				aStream) const
				{
					aStream->WriteObjects(m_entries);
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if(!aStream->ReadObjects(m_entries))
						return false;
					return true;
				}

				ResourceEntry*
				GetResourceEntry(
					uint32_t				aResourceId)
				{
					for(ResourceEntry& t : m_entries)
					{
						if(t.m_id == aResourceId)
							return &t;
					}
					return NULL;
				}

				const ResourceEntry*
				GetResourceEntry(
					uint32_t				aResourceId) const
				{
					for(const ResourceEntry& t : m_entries)
					{
						if(t.m_id == aResourceId)
							return &t;
					}
					return NULL;
				}

				// Public data
				std::vector<ResourceEntry>			m_entries;
			};

			struct DespawnTime
			{
				void
				FromSource(
					const SourceNode*		aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if (aChild->m_name == "without_loot_ticks")
							m_ticksWithoutLoot = aChild->GetInt32();
						else if (aChild->m_name == "with_loot_ticks")
							m_ticksWithLoot = aChild->GetInt32();
						else if (aChild->m_name == "out_of_combat_ticks")
							m_ticksOutOfCombat = aChild->GetInt32();
						else if (aChild->m_name == "without_loot_mins")
							m_ticksWithoutLoot = aChild->GetInt32() * 10 * 60;
						else if (aChild->m_name == "with_loot_mins")
							m_ticksWithLoot = aChild->GetInt32() * 10 * 60;
						else if (aChild->m_name == "out_of_combat_mins")
							m_ticksOutOfCombat = aChild->GetInt32() * 10 * 60;
						else if (aChild->m_name == "immediately_when_dead")
							m_immediatelyWhenDead = aChild->GetBool();
						else if (aChild->m_name == "never")
							m_never = aChild->GetBool();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*				aStream) const
				{
					aStream->WriteInt(m_ticksWithoutLoot);
					aStream->WriteInt(m_ticksWithLoot);
					aStream->WriteInt(m_ticksOutOfCombat);
					aStream->WriteBool(m_immediatelyWhenDead);
					aStream->WriteBool(m_never);
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if(!aStream->ReadInt(m_ticksWithoutLoot))
						return false;
					if (!aStream->ReadInt(m_ticksWithLoot))
						return false;
					if (!aStream->ReadInt(m_ticksOutOfCombat))
						return false;
					if (!aStream->ReadBool(m_immediatelyWhenDead))
						return false;
					if (!aStream->ReadBool(m_never))
						return false;
					return true;
				}

				// Public data
				int32_t								m_ticksWithoutLoot = 2 * 60 * 10; // 2 min
				int32_t								m_ticksWithLoot = 5 * 60 * 10; // 5 min
				int32_t								m_ticksOutOfCombat = 0; // Disabled
				bool								m_immediatelyWhenDead = false;
				bool								m_never = false;
			};

			enum Field
			{
				FIELD_STATES,
				FIELD_RESOURCES,
				FIELD_DESPAWN_TIME,
				FIELD_DEFAULT_BEHAVIOR_STATE,
				FIELD_CAN_MOVE_ON_ALL_NON_VIEW_BLOCKING_TILES,
				FIELD_BLOCKING,
				FIELD_ENCOUNTER,
				FIELD_INACTIVE_ENCOUNTER_DESPAWN,
				FIELD_LARGE,
				FIELD_ROUTE,
				FIELD_ZONE,
				FIELD_OUT_OF_ZONE_ACTION,
				FIELD_CAN_MOVE,
				FIELD_EVADE_DESPAWN,
				FIELD_AGGRO_REQUIREMENTS,
				FIELD_CAN_DIE,
				FIELD_MAX_LEASH_DISTANCE,
				FIELD_MELEE_PUSH_PRIORITY,
				FIELD_OTHER_NPC_PUSH_OVERRIDE,
				FIELD_DISPLAY_NAME_WHEN_DEAD,
				FIELD_INACTIVE_ENCOUNTER_DESPAWN_STATE,
				FIELD_NO_KILL_EVENT,
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->DefineCustomObjectPointersSingleAppend<StateEntry>(FIELD_STATES, "state", offsetof(NPC, m_states));
				aSchema->DefineCustomObject<Resources>(FIELD_RESOURCES, "resources", offsetof(NPC, m_resources));
				aSchema->DefineCustomObject<DespawnTime>(FIELD_DESPAWN_TIME, "despawn_time", offsetof(NPC, m_despawnTime));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_DEFAULT_BEHAVIOR_STATE, "default_behavior_state", offsetof(NPC, m_defaultBehaviorStateId))->SetDataType(DataType::ID_NPC_BEHAVIOR_STATE);
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_CAN_MOVE_ON_ALL_NON_VIEW_BLOCKING_TILES, "can_move_on_all_non_view_blocking_tiles", offsetof(NPC, m_canMoveOnAllNonViewBlockingTiles));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_BLOCKING, "blocking", offsetof(NPC, m_blocking));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_ENCOUNTER, "encounter", offsetof(NPC, m_encounterId))->SetDataType(DataType::ID_ENCOUNTER);
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_INACTIVE_ENCOUNTER_DESPAWN, "inactive_encounter_despawn", offsetof(NPC, m_inactiveEncounterDespawn));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_BLOCKING, "large", offsetof(NPC, m_large));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_CAN_MOVE, "can_move", offsetof(NPC, m_canMove));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_ROUTE, "route", offsetof(NPC, m_routeId))->SetDataType(DataType::ID_ROUTE);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_ZONE, "zone", offsetof(NPC, m_zoneId))->SetDataType(DataType::ID_ZONE);
				aSchema->DefineCustomObject<OutOfZoneAction>(FIELD_OUT_OF_ZONE_ACTION, "out_of_zone_action", offsetof(NPC, m_outOfZoneAction));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_EVADE_DESPAWN, "evade_despawn", offsetof(NPC, m_evadeDespawn));
				aSchema->DefineCustomObject<AggroRequirements>(FIELD_AGGRO_REQUIREMENTS, "aggro_requirements", offsetof(NPC, m_aggroRequirements));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_CAN_DIE, "can_die", offsetof(NPC, m_canDie));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_MAX_LEASH_DISTANCE, "max_leash_distance", offsetof(NPC, m_maxLeashDistance));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_MELEE_PUSH_PRIORITY, "melee_push_priority", offsetof(NPC, m_meleePushPriority));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_OTHER_NPC_PUSH_OVERRIDE, "other_npc_push_override", offsetof(NPC, m_otherNPCPushOverride));
				aSchema->Define(ComponentSchema::TYPE_STRING, FIELD_DISPLAY_NAME_WHEN_DEAD, "display_name_when_dead", offsetof(NPC, m_displayNameWhenDead));
				aSchema->Define(ComponentSchema::TYPE_UINT8, FIELD_INACTIVE_ENCOUNTER_DESPAWN_STATE, "inactive_encounter_despawn_state", offsetof(NPC, m_inactiveEncounterDespawnState))->SetFlags(ComponentSchema::FLAG_ENTITY_STATE);
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_NO_KILL_EVENT, "no_kill_event", offsetof(NPC, m_noKillEvent));
			}

			const StateEntry*
			GetState(
				EntityState::Id			aEntityState) const
			{
				for(const std::unique_ptr<StateEntry>& t : m_states)
				{
					if(t->m_entityState == aEntityState)
						return t.get();
				}
				return NULL;
			}

			void
			Reset()
			{
				m_states.clear();
				m_resources.m_entries.clear();
				m_despawnTime = DespawnTime();
				m_defaultBehaviorStateId = 0;
				m_canMoveOnAllNonViewBlockingTiles = false;
				m_blocking = true;
				m_large = false;
				m_canMove = true;
				m_inactiveEncounterDespawn = false;
				m_encounterId = 0;
				m_routeId = 0;
				m_zoneId = 0;
				m_outOfZoneAction = OutOfZoneAction();
				m_evadeDespawn = false;
				m_aggroRequirements = AggroRequirements();
				m_canDie = true;
				m_maxLeashDistance = 0;
				m_meleePushPriority = 0;
				m_otherNPCPushOverride = false;
				m_displayNameWhenDead.clear();
				m_inactiveEncounterDespawnState = EntityState::INVALID_ID;
				m_noKillEvent = false;

				m_cooldowns.m_entries.clear();
				m_castInProgress.reset();
				m_channeling.reset();
				m_targetEntity = SourceEntityInstance();
				m_moveCooldownUntilTick = 0;
				m_spawnPosition = Vec2();
				m_anchorPosition = Vec2();
				m_npcBehaviorState = NULL;
				m_npcBehaviorStateTick = 0;
				m_npcMovement.Reset(0);
				m_restoreResources = false;
				m_spawnWithTarget.reset();
				m_lastAttackTick = 0;
				m_lastCombatMoveTick = 0;
				m_routeIsReversing = false;
				m_subRouteIndex = SIZE_MAX;
				m_lastTargetPosition = Vec2();
				m_handledRouteTriggerIndices.clear();
				m_effectiveRouteId = 0;
				m_patrolResetAfterLeavingCombat = false;
			}

			// Public data			
			std::vector<std::unique_ptr<StateEntry>>	m_states;
			Resources									m_resources;
			DespawnTime									m_despawnTime;
			uint32_t									m_defaultBehaviorStateId = 0;
			bool										m_canMoveOnAllNonViewBlockingTiles = false;
			bool										m_blocking = true;
			bool										m_large = false;
			bool										m_inactiveEncounterDespawn = false;
			bool										m_canDie = true;
			bool										m_canMove = true;
			uint32_t									m_encounterId = 0;
			uint32_t									m_routeId = 0;
			uint32_t									m_zoneId = 0;
			OutOfZoneAction								m_outOfZoneAction;
			bool										m_evadeDespawn = false;
			AggroRequirements							m_aggroRequirements;
			uint32_t									m_maxLeashDistance = 0;
			uint32_t									m_meleePushPriority = 0;
			bool										m_otherNPCPushOverride = false;
			std::string									m_displayNameWhenDead;
			EntityState::Id								m_inactiveEncounterDespawnState = EntityState::INVALID_ID;
			bool										m_noKillEvent = false;

			// Not serialized
			Cooldowns									m_cooldowns;
			std::optional<CastInProgress>				m_castInProgress;
			std::optional<CastInProgress>				m_channeling;
			SourceEntityInstance						m_targetEntity;
			int32_t										m_moveCooldownUntilTick = 0;
			Vec2										m_spawnPosition;
			Vec2										m_anchorPosition;
			const Data::NPCBehaviorState*				m_npcBehaviorState = NULL;
			uint32_t									m_npcBehaviorStateTick = 0;
			NPCMovement									m_npcMovement;			
			bool										m_restoreResources = false;
			int32_t										m_lastAttackTick = 0;
			int32_t										m_lastCombatMoveTick = 0;
			bool										m_routeIsReversing = false;
			size_t										m_subRouteIndex = SIZE_MAX;
			uint32_t									m_effectiveRouteId = 0;
			Vec2										m_lastTargetPosition;
			bool										m_patrolResetAfterLeavingCombat = false;

			struct SpawnWithTarget
			{
				SourceEntityInstance					m_entity;
				int32_t									m_threat = 0;
			};

			std::optional<SpawnWithTarget>				m_spawnWithTarget;
			std::unordered_set<uint32_t>				m_handledRouteTriggerIndices;
		};
	}

}
