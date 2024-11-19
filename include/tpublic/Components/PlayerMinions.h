#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct PlayerMinions
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_MINIONS;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			struct MinionData
			{
				void
				ToStream(
					IWriter*		aWriter) const
				{
					aWriter->WriteUInt(m_armor);
					aWriter->WriteUInt(m_health);
					aWriter->WriteUInt(m_mana);
					aWriter->WriteUInt(m_weaponDamageMin);
					aWriter->WriteUInt(m_weaponDamageMax);
					aWriter->WriteUInt(m_level);
				}

				bool
				FromStream(
					IReader*		aReader) 
				{
					if (!aReader->ReadUInt(m_armor))
						return false;
					if (!aReader->ReadUInt(m_health))
						return false;
					if (!aReader->ReadUInt(m_mana))
						return false;
					if (!aReader->ReadUInt(m_weaponDamageMin))
						return false;
					if (!aReader->ReadUInt(m_weaponDamageMax))
						return false;
					if (!aReader->ReadUInt(m_level))
						return false;
					return true;
				}

				// Public data
				uint32_t		m_armor = 0;
				uint32_t		m_health = 0;
				uint32_t		m_mana = 0;
				uint32_t		m_weaponDamageMin = 0;
				uint32_t		m_weaponDamageMax = 0;
				uint32_t		m_level = 0;
			};

			struct Minion
			{
				static const uint8_t VERSION = 2;

				void
				ToStream(
					IWriter*		aWriter) const 
				{
					aWriter->WriteUInt(m_entityId);
					aWriter->WriteUInt(m_entityInstanceId);

					// Hack to support new and old data. Should have thought about this before. :)
					uint8_t header = 0x80 | (m_dead ? 0x01 : 0x00);
					aWriter->WritePOD(header);
					aWriter->WritePOD(VERSION);

					aWriter->WriteUInt(m_spawnTimeStamp);
					aWriter->WriteUInt(m_durationSeconds);
					aWriter->WriteOptionalObject(m_data);
				}

				bool
				FromStream(
					IReader*		aReader) 
				{
					if (!aReader->ReadUInt(m_entityId))
						return false;
					if (!aReader->ReadUInt(m_entityInstanceId))
						return false;

					uint8_t header;
					if(!aReader->ReadPOD(header))
						return false;
					m_dead = (header & 0x01) != 0;

					if(header & 0x80)
					{
						// We have extra minion info
						uint8_t version;
						if(!aReader->ReadPOD(version))
							return false;

						if(version > VERSION)
							return false;

						if (!aReader->ReadUInt(m_spawnTimeStamp))
							return false;

						if(version >= 1)
						{
							if (!aReader->ReadUInt(m_durationSeconds))
								return false;
						}

						if(version >= 2)
						{
							if(!aReader->ReadOptionalObject(m_data))
								return false;
						}
					}

					return true;
				}

				// Public data
				uint32_t					m_entityId = 0;
				uint32_t					m_entityInstanceId = 0;
				bool						m_dead = false;
				
				// Extra minion info
				uint64_t					m_spawnTimeStamp = 0;
				uint32_t					m_durationSeconds = 0;
				std::optional<MinionData>	m_data;

				// Internal
				bool						m_applyControl = false;
			};

			struct MinionControl
			{
				void
				ToStream(
					IWriter*		aWriter) const 
				{
					aWriter->WriteUInt(m_entityId);
					aWriter->WriteUInts(m_blockedAbilityIds);
					aWriter->WriteUInt(m_currentMinionModeId);
				}

				bool
				FromStream(
					IReader*		aReader) 
				{
					if (!aReader->ReadUInt(m_entityId))
						return false;
					if (!aReader->ReadUInts(m_blockedAbilityIds))
						return false;
					if (!aReader->ReadUInt(m_currentMinionModeId))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_entityId = 0;
				std::vector<uint32_t>	m_blockedAbilityIds;
				uint32_t				m_currentMinionModeId = 0;
			};

			enum Field
			{
				FIELD_MINIONS,
				FIELD_MINION_CONTROL,
				FIELD_INSTANCE_MINION_LIMIT_REACHED
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->DefineCustomObjectsNoSource<Minion>(FIELD_MINIONS, offsetof(PlayerMinions, m_minions));
				aSchema->DefineCustomObjectsNoSource<MinionControl>(FIELD_MINION_CONTROL, offsetof(PlayerMinions, m_minionControl));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_INSTANCE_MINION_LIMIT_REACHED, NULL, offsetof(PlayerMinions, m_instanceMinionLimitReached))->SetFlags(ComponentSchema::FLAG_NO_STORAGE);
			}

			void
			Reset()
			{
				m_minions.clear();
				m_minionControl.clear();
				m_instanceMinionLimitReached = false;
			}

			bool
			HasMinion(
				uint32_t			aEntityInstanceId) const
			{
				for(const Minion& t : m_minions)
				{
					if(t.m_entityInstanceId == aEntityInstanceId)
						return true;
				}
				return false;
			}

			void
			RemoveMinion(
				uint32_t			aEntityInstanceId)
			{
				for(size_t i = 0; i < m_minions.size(); i++)
				{
					if(m_minions[i].m_entityInstanceId == aEntityInstanceId)
					{
						m_minions.erase(m_minions.begin() + i);
						return;
					}
				}
			}

			MinionControl*
			GetMinionControl(
				uint32_t			aEntityId)
			{
				for(MinionControl& minionControl : m_minionControl)
				{
					if(minionControl.m_entityId == aEntityId)
						return &minionControl;
				}
				MinionControl t;
				t.m_entityId = aEntityId;
				m_minionControl.push_back(t);
				return &m_minionControl[m_minionControl.size() - 1];
			}

			const Minion*
			GetMinion(
				uint32_t			aEntityInstanceId) const
			{
				for (const Minion& t : m_minions)
				{
					if (t.m_entityInstanceId == aEntityInstanceId)
						return &t;
				}
				return NULL;
			}

			// Public data
			std::vector<Minion>			m_minions;
			std::vector<MinionControl>	m_minionControl;
			bool						m_instanceMinionLimitReached = false;

			// Internal
			std::vector<uint32_t>		m_reimburseItemIds;
		};
	}

}