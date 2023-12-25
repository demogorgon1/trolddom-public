#pragma once

#include "../Component.h"
#include "../ComponentBase.h"
#include "../EventHistory.h"
#include "../PlayerProfessions.h"
#include "../Vec2.h"

namespace tpublic
{

	namespace Components
	{

		struct PlayerPrivate
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_PRIVATE;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;
			
			struct GuildRegistrationHistory
			{
				GuildRegistrationHistory()
					: m_eventHistory(7 * 24 * 60 * 60) // Count guild registrations within last 7 days
				{

				}

				void
				ToStream(
					IWriter*	aWriter) const
				{
					m_eventHistory.ToStream(aWriter);
				}

				bool
				FromStream(
					IReader*	aReader)
				{
					if(!m_eventHistory.FromStream(aReader))
						return false;
					return true;
				}

				// Public data
				EventHistory<3>		m_eventHistory;
			};

			enum Field
			{
				FIELD_LEVEL,
				FIELD_XP,
				FIELD_IS_DEAD,
				FIELD_RESURRECTION_POINT_POSITION,
				FIELD_RESURRECTION_POINT_MAP_ID,
				FIELD_GUILD_REGISTRATION_HISTORY,
				FIELD_PROFESSIONS
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_LEVEL, NULL, offsetof(PlayerPrivate, m_level));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_XP, NULL, offsetof(PlayerPrivate, m_xp));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_IS_DEAD, NULL, offsetof(PlayerPrivate, m_isDead));
				aSchema->Define(ComponentSchema::TYPE_VEC2, FIELD_RESURRECTION_POINT_POSITION, NULL, offsetof(PlayerPrivate, m_resurrectionPointPosition));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_RESURRECTION_POINT_MAP_ID, NULL, offsetof(PlayerPrivate, m_resurrectionPointMapId));
				aSchema->DefineCustomObjectNoSource<GuildRegistrationHistory>(FIELD_GUILD_REGISTRATION_HISTORY, offsetof(PlayerPrivate, m_guildRegistrationHistory));
				aSchema->DefineCustomObjectNoSource<PlayerProfessions>(FIELD_PROFESSIONS, offsetof(PlayerPrivate, m_professions));
			}

			// Public data
			uint32_t										m_level = 1;
			uint32_t										m_xp = 0;
			bool											m_isDead = false;
			Vec2											m_resurrectionPointPosition;
			uint32_t										m_resurrectionPointMapId = 0;
			GuildRegistrationHistory						m_guildRegistrationHistory;
			PlayerProfessions								m_professions;

			// Not serialized
			bool											m_positionUpdatedOnServer = false;
			bool											m_pvpCombatEvent = false;
			int32_t											m_lastPVPCombatEventTick = 0;
		};
	}

}