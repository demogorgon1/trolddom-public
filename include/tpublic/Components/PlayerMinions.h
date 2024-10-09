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

			struct Minion
			{
				void
				ToStream(
					IWriter*		aWriter) const 
				{
					aWriter->WriteUInt(m_entityId);
					aWriter->WriteUInt(m_entityInstanceId);
					aWriter->WriteBool(m_dead);
				}

				bool
				FromStream(
					IReader*		aReader) 
				{
					if (!aReader->ReadUInt(m_entityId))
						return false;
					if (!aReader->ReadUInt(m_entityInstanceId))
						return false;
					if(!aReader->ReadBool(m_dead))
						return false;
					return true;
				}

				// Public data
				uint32_t		m_entityId = 0;
				uint32_t		m_entityInstanceId = 0;
				bool			m_dead = false;
			};

			enum Field
			{
				FIELD_MINIONS
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->DefineCustomObjectsNoSource<Minion>(FIELD_MINIONS, offsetof(PlayerMinions, m_minions));
			}

			void
			Reset()
			{
				m_minions.clear();
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

			// Public data
			std::vector<Minion>	m_minions;
		};
	}

}