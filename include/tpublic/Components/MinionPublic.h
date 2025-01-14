#pragma once

#include "../Cooldowns.h"
#include "../Component.h"
#include "../ComponentBase.h"
#include "../MinionCommand.h"
#include "../Vec2.h"

namespace tpublic
{

	namespace Components
	{

		struct MinionPublic
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_MINION_PUBLIC;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			struct Ability
			{
				enum Flag : uint8_t
				{
					FLAG_BLOCKED	= 0x01,
					FLAG_AUTO		= 0x02
				};

				static uint8_t
				SourceToFlags(
					const SourceNode*	aSource)
				{
					uint8_t t = 0;

					aSource->GetArray()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->IsIdentifier("blocked"))
							t |= FLAG_BLOCKED;
						else if (aChild->IsIdentifier("auto"))
							t |= FLAG_AUTO;
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid flag.", aChild->GetIdentifier());
					});

					return t;
				}

				Ability()
				{

				}

				void
				FromSource(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "flags")
							m_flags = SourceToFlags(aChild);
						else if(aChild->m_name == "id")
							m_id = aChild->GetId(DataType::ID_ABILITY);
						else if (aChild->m_name == "max_target_health")
							m_maxTargetHealth = aChild->GetUInt32();
						else if (aChild->m_name == "min_neighbor_hostiles")
							m_minNeighborHostiles = aChild->GetUInt32();
						else if (aChild->m_name == "target_must_not_have_aura")
							m_targetMustNotHaveAuraId = aChild->GetId(DataType::ID_AURA);
						else if (aChild->m_name == "self_must_not_have_aura")
							m_selfMustNotHaveAuraId = aChild->GetId(DataType::ID_AURA);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_id);
					aWriter->WritePOD(m_flags);
					aWriter->WriteUInt(m_maxTargetHealth);
					aWriter->WriteUInt(m_targetMustNotHaveAuraId);
					aWriter->WriteUInt(m_selfMustNotHaveAuraId);
					aWriter->WriteUInt(m_minNeighborHostiles);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if (!aReader->ReadUInt(m_id))
						return false;
					if(!aReader->ReadPOD(m_flags))
						return false;
					if (!aReader->ReadUInt(m_maxTargetHealth))
						return false;
					if (!aReader->ReadUInt(m_targetMustNotHaveAuraId))
						return false;
					if (!aReader->ReadUInt(m_selfMustNotHaveAuraId))
						return false;
					if (!aReader->ReadUInt(m_minNeighborHostiles))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_id = 0;
				uint8_t				m_flags = 0;
				uint32_t			m_maxTargetHealth = 0;
				uint32_t			m_targetMustNotHaveAuraId = 0;
				uint32_t			m_selfMustNotHaveAuraId = 0;
				uint32_t			m_minNeighborHostiles = 0;
			};

			struct Command
			{
				Command()
				{

				}

				void
				FromSource(
					const SourceNode*	aSource)
				{
					m_id = MinionCommand::StringToId(aSource->GetIdentifier());
					TP_VERIFY(m_id != MinionCommand::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid minion command.", aSource->GetIdentifier());
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_id);
					aWriter->WriteBool(m_active);
					aWriter->WriteUInt(m_targetEntityInstanceId);
					m_targetPosition.ToStream(aWriter);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!aReader->ReadPOD(m_id))
						return false;
					if(!aReader->ReadBool(m_active))
						return false;
					if(!aReader->ReadUInt(m_targetEntityInstanceId))
						return false;
					if(!m_targetPosition.FromStream(aReader))
						return false;
					return true;
				}

				// Public data
				MinionCommand::Id	m_id = MinionCommand::INVALID_ID;
				bool				m_active = false;
				uint32_t			m_targetEntityInstanceId = 0;
				Vec2				m_targetPosition;

				// Internal
				bool				m_serverActive = false;
			};

			enum Field
			{
				FIELD_OWNER_ENTITY_INSTANCE,
				FIELD_CURRENT_MINION_MODE,
				FIELD_MINION_MODES,
				FIELD_COMMANDS,
				FIELD_COOLDOWNS,
				FIELD_ABILITIES,
				FIELD_DURATION_SECONDS,
				FIELD_SPAWN_TIMESTAMP

			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_OWNER_ENTITY_INSTANCE, NULL, offsetof(MinionPublic, m_ownerEntityInstanceId));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_CURRENT_MINION_MODE, "current_minion_mode", offsetof(MinionPublic, m_currentMinionModeId))->SetDataType(DataType::ID_MINION_MODE);
				aSchema->Define(ComponentSchema::TYPE_UINT32_ARRAY, FIELD_MINION_MODES, "minion_modes", offsetof(MinionPublic, m_minionModeIds))->SetDataType(DataType::ID_MINION_MODE);
				aSchema->DefineCustomObjects<Command>(FIELD_COMMANDS, "commands", offsetof(MinionPublic, m_commands));
				aSchema->DefineCustomObjectNoSource<Cooldowns>(FIELD_COOLDOWNS, offsetof(MinionPublic, m_cooldowns));
				aSchema->DefineCustomObjects<Ability>(FIELD_ABILITIES, "abilities", offsetof(MinionPublic, m_abilities));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_DURATION_SECONDS, "duration_seconds", offsetof(MinionPublic, m_durationSeconds));
				aSchema->Define(ComponentSchema::TYPE_UINT64, FIELD_SPAWN_TIMESTAMP, NULL, offsetof(MinionPublic, m_spawnTimeStamp));
			}

			void
			Reset()
			{
				m_ownerEntityInstanceId = 0;
				m_currentMinionModeId = 0;
				m_minionModeIds.clear();
				m_commands.clear();
				m_cooldowns.m_entries.clear();
				m_abilities.clear();
				m_durationSeconds = 0;
				m_spawnTimeStamp = 0;

				m_ownerThreatTargetEntityInstanceId = 0;
			}

			bool
			HasMinionMode(
				uint32_t			aMinionModeId) const
			{
				for(uint32_t minionModeId : m_minionModeIds)
				{
					if(minionModeId == aMinionModeId)
						return true;
				}
				return false;
			}

			bool
			IsCommandActive(
				MinionCommand::Id	aMinionCommandId) const
			{
				for(const Command& command : m_commands)
				{
					if(command.m_id == aMinionCommandId)
						return command.m_active;
				}
				return false;
			}

			Command*
			GetCommand(
				MinionCommand::Id	aMinionCommandId)
			{
				for (Command& command : m_commands)
				{
					if (command.m_id == aMinionCommandId)
						return &command;
				}
				return NULL;
			}

			const Command*
			GetCommand(
				MinionCommand::Id	aMinionCommandId) const
			{
				for (const Command& command : m_commands)
				{
					if (command.m_id == aMinionCommandId)
						return &command;
				}
				return NULL;
			}

			bool
			HasAbility(
				uint32_t			aAbilityId) const
			{
				for(const Ability& ability : m_abilities)
				{
					if(ability.m_id == aAbilityId)
						return true;
				}
				return false;
			}

			const Ability*
			GetAbility(
				uint32_t			aAbilityId) const
			{
				for(const Ability& ability : m_abilities)
				{
					if(ability.m_id == aAbilityId)
						return &ability;
				}
				return NULL;
			}

			Ability*
			GetAbility(
				uint32_t			aAbilityId) 
			{
				for(Ability& ability : m_abilities)
				{
					if(ability.m_id == aAbilityId)
						return &ability;
				}
				return NULL;
			}

			bool
			IsAbilityBlocked(
				uint32_t			aAbilityId) const
			{
				for(const Ability& ability : m_abilities)
				{
					if(ability.m_id == aAbilityId)
						return (ability.m_flags & Ability::FLAG_BLOCKED) != 0 || (ability.m_flags & Ability::FLAG_AUTO) == 0;
				}
				return false;
			}

			void
			GetBlockedAbilities(
				std::vector<uint32_t>&	aOut) const
			{
				for (const Ability& ability : m_abilities)
				{
					if (ability.m_flags & Ability::FLAG_BLOCKED)
						aOut.push_back(ability.m_id);
				}
			}

			// Public data
			uint32_t				m_ownerEntityInstanceId = 0;
			uint32_t				m_currentMinionModeId = 0;
			std::vector<uint32_t>	m_minionModeIds;
			std::vector<Command>	m_commands;
			Cooldowns				m_cooldowns;
			std::vector<Ability>	m_abilities;
			uint32_t				m_durationSeconds = 0;
			uint64_t				m_spawnTimeStamp = 0;

			// Internal
			uint32_t				m_ownerThreatTargetEntityInstanceId = 0;
		};
	}

}