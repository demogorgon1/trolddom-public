#pragma once

#include "../Component.h"
#include "../Cooldowns.h"
#include "../EntityState.h"
#include "../Stat.h"

namespace kaos_public
{

	namespace Components
	{

		struct NPC
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_NPC;
			static const uint8_t FLAGS = 0;

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
							KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
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
					KP_VERIFY(m_entityState != EntityState::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid entity state.", aSource->GetIdentifier());

					aSource->GetObject()->ForEachChild([&](
						const Parser::Node* aChild)
					{
						if(aChild->m_name == "sprite")
						{
							m_spriteId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->GetIdentifier());
						}
						else if(aChild->m_name == "abilities")
						{
							aChild->GetArray()->ForEachChild([&](
								const Parser::Node* aAbility)
							{
								m_abilities.push_back(AbilityEntry(aAbility));
							});
						}
						else 
						{
							KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
						}
					});
				}

				void
				ToStream(
					IWriter*				aStream) const
				{
					aStream->WritePOD(m_entityState);
					aStream->WriteUInt(m_spriteId);
					aStream->WriteObjects(m_abilities);
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if(!aStream->ReadPOD(m_entityState))
						return false;
					if(!aStream->ReadUInt(m_spriteId))
						return false;
					if(!aStream->ReadObjects(m_abilities))
						return false;
					return true;
				}

				// Public data
				EntityState::Id						m_entityState = EntityState::ID_DEFAULT;
				uint32_t							m_spriteId = 0;
				std::vector<AbilityEntry>			m_abilities;
			};

			NPC()
				: ComponentBase(ID, FLAGS)
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
						m_states.push_back(std::make_unique<StateEntry>(aChild));
					else if (aChild->m_name == "stats")
						m_stats.FromSource(aChild);
					else
						KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteObjectPointers(m_states);
				m_stats.ToStream(aStream);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadObjectPointers(m_states))
					return false;
				if(!m_stats.FromStream(aStream))
					return false;
				return true;
			}

			// Public data			
			Stat::Collection							m_stats;
			std::vector<std::unique_ptr<StateEntry>>	m_states;
			
			Cooldowns									m_cooldowns;
		};
	}

}