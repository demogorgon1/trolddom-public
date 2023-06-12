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
				StateEntry()
				{

				}

				StateEntry(
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
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if(!aStream->ReadPOD(m_entityState))
						return false;
					if(!aStream->ReadObjects(m_abilities))
						return false;
					return true;
				}

				// Public data
				EntityState::Id						m_entityState = EntityState::ID_DEFAULT;
				std::vector<AbilityEntry>			m_abilities;
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

			NPC()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual  
			~NPC()
			{

			}

			const StateEntry*
			GetState(
				EntityState::Id			aEntityState)
			{
				for(const std::unique_ptr<StateEntry>& t : m_states)
				{
					if(t->m_entityState == aEntityState)
						return t.get();
				}
				return NULL;
			}

			// ComponentBase implementation
			void	
			FromSource(
				const Parser::Node*		aSource) override
			{
				aSource->ForEachChild([&](
					const Parser::Node*	aChild)
				{
					if (aChild->m_tag == "state")
					{
						m_states.push_back(std::make_unique<StateEntry>(aChild));
					}
					else if (aChild->m_name == "resources")
					{
						aChild->ForEachChild([&](
							const Parser::Node* aResource)
						{
							ResourceEntry t;
							t.m_id = Resource::StringToId(aResource->m_name.c_str());
							TP_VERIFY(t.m_id != 0, aResource->m_debugInfo, "'%s' is not a valid resource.", aResource->m_name.c_str());
							t.m_max = aResource->GetUInt32();
							m_resources.push_back(t);
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
				IWriter*				aStream) const override
			{
				aStream->WriteObjectPointers(m_states);
				aStream->WriteObjects(m_resources);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadObjectPointers(m_states))
					return false;
				if(!aStream->ReadObjects(m_resources))
					return false;
				return true;
			}

			// Public data			
			std::vector<std::unique_ptr<StateEntry>>	m_states;
			std::vector<ResourceEntry>					m_resources;
			
			Cooldowns									m_cooldowns;
			std::optional<CastInProgress>				m_castInProgress;
			uint32_t									m_targetEntityInstanceId = 0;
			int32_t										m_moveCooldownUntilTick = 0;
		};
	}

}