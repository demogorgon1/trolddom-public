#pragma once

#include "../Data/NPCBehaviorState.h"

#include "../CastInProgress.h"
#include "../Component.h"
#include "../Cooldowns.h"
#include "../DialogueScript.h"
#include "../EntityState.h"
#include "../NPCMovement.h"
#include "../Resource.h"

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

			struct AbilityEntry
			{
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
							m_abilityId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY, aChild->GetIdentifier());
						else if(aChild->m_name == "use_probability")
							m_useProbability = aChild->GetProbability();
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
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if(!aStream->ReadUInt(m_abilityId))
						return false;
					if(!aStream->ReadUInt(m_useProbability))
						return false;
					return true;
				}	

				// Public data
				uint32_t							m_abilityId = 0;
				uint32_t							m_useProbability = UINT32_MAX;
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
						else if(aChild->m_name == "dialogue_scripts")
						{
							aChild->GetIdArrayWithLookup<DialogueScript::Id, DialogueScript::INVALID_ID>(m_dialogueScripts, [](
								const char* aIdentifier) -> DialogueScript::Id
							{
								return DialogueScript::StringToId(aIdentifier);
							});
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
						else if (aChild->m_name == "without_loot_mins")
							m_ticksWithoutLoot = aChild->GetInt32() * 10 * 60;
						else if (aChild->m_name == "with_loot_mins")
							m_ticksWithLoot = aChild->GetInt32() * 10 * 60;
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
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if(!aStream->ReadInt(m_ticksWithoutLoot))
						return false;
					if (!aStream->ReadInt(m_ticksWithLoot))
						return false;
					return true;
				}
				// Public data
				int32_t								m_ticksWithoutLoot = 2 * 60 * 10; // 2 min
				int32_t								m_ticksWithLoot = 5 * 60 * 10; // 5 min
			};

			enum Field
			{
				FIELD_STATES,
				FIELD_RESOURCES,
				FIELD_DESPAWN_TIME,
				FIELD_DEFAULT_BEHAVIOR_STATE,
				FIELD_CAN_MOVE_ON_ALL_NON_VIEW_BLOCKING_TILES
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->DefineCustomObjectPointersSingleAppend<StateEntry>(FIELD_STATES, "state", offsetof(NPC, m_states));
				aSchema->DefineCustomObject<Resources>(FIELD_RESOURCES, "resources", offsetof(NPC, m_resources));
				aSchema->DefineCustomObject<DespawnTime>(FIELD_DESPAWN_TIME, "despawn_time", offsetof(NPC, m_despawnTime));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_DEFAULT_BEHAVIOR_STATE, "default_behavior_state", offsetof(NPC, m_defaultBehaviorState))->SetDataType(DataType::ID_NPC_BEHAVIOR_STATE);
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_CAN_MOVE_ON_ALL_NON_VIEW_BLOCKING_TILES, "can_move_on_all_non_view_blocking_tiles", offsetof(NPC, m_canMoveOnAllNonViewBlockingTiles));
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

			// Public data			
			std::vector<std::unique_ptr<StateEntry>>	m_states;
			Resources									m_resources;
			DespawnTime									m_despawnTime;
			uint32_t									m_defaultBehaviorState = 0;
			bool										m_canMoveOnAllNonViewBlockingTiles = false;

			// Not serialized
			Cooldowns									m_cooldowns;
			std::optional<CastInProgress>				m_castInProgress;
			uint32_t									m_targetEntityInstanceId = 0;
			int32_t										m_moveCooldownUntilTick = 0;
			Vec2										m_spawnPosition;
			Vec2										m_anchorPosition;
			const Data::NPCBehaviorState*				m_npcBehaviorState = NULL;
			uint32_t									m_npcBehaviorStateTick = 0;
			NPCMovement									m_npcMovement;
		};
	}

}