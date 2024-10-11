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
				FIELD_ABILITIES
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
				aSchema->Define(ComponentSchema::TYPE_UINT32_ARRAY, FIELD_ABILITIES, "abilities", offsetof(MinionPublic, m_abilities))->SetDataType(DataType::ID_ABILITY);
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

			// Public data
			uint32_t				m_ownerEntityInstanceId = 0;
			uint32_t				m_currentMinionModeId = 0;
			std::vector<uint32_t>	m_minionModeIds;
			std::vector<Command>	m_commands;
			Cooldowns				m_cooldowns;
			std::vector<uint32_t>	m_abilities;

			// Internal
			uint32_t				m_ownerThreatTargetEntityInstanceId = 0;
		};
	}

}