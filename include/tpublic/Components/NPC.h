#pragma once

#include "../CastInProgress.h"
#include "../Component.h"
#include "../Cooldowns.h"
#include "../EntityState.h"
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

			struct AbilityEntry
			{
				AbilityEntry()
				{

				}

				AbilityEntry(
					const Parser::Node*		aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const Parser::Node* aChild)
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
					const Parser::Node*		aSource)
				{
					m_entityState = EntityState::StringToId(aSource->m_name.c_str());
					TP_VERIFY(m_entityState != EntityState::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid entity state.", aSource->GetIdentifier());

					aSource->GetObject()->ForEachChild([&](
						const Parser::Node* aChild)
					{
						if(aChild->m_name == "abilities")
						{
							aChild->GetArray()->ForEachChild([&](
								const Parser::Node* aAbility)
							{
								m_abilities.push_back(AbilityEntry(aAbility));
							});
						}
						else if(aChild->m_name == "dialogue_scripts")
						{
							aChild->GetIdArrayWithLookup<DialogueScript::Id, DialogueScript::INVALID_ID>(m_dialogueScripts, [aChild](
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
					const Parser::Node*		aSource)
				{
					aSource->ForEachChild([&](
						const Parser::Node* aChild)
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

				// Public data
				std::vector<ResourceEntry>			m_entries;
			};

			enum Field
			{
				FIELD_STATES,
				FIELD_RESOURCES
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->DefineCustomObjectPointersSingleAppend<StateEntry>(FIELD_STATES, "state", offsetof(NPC, m_states));
				aSchema->DefineCustomObject<Resources>(FIELD_RESOURCES, "resources", offsetof(NPC, m_resources));
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
			
			// Not serialized
			Cooldowns									m_cooldowns;
			std::optional<CastInProgress>				m_castInProgress;
			uint32_t									m_targetEntityInstanceId = 0;
			int32_t										m_moveCooldownUntilTick = 0;
		};
	}

}