#pragma once

#include "../Component.h"
#include "../EventHistory.h"

namespace tpublic
{

	namespace Components
	{

		struct PlayerPrivate
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_PRIVATE;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			
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
				FIELD_GUILD_REGISTRATION_HISTORY
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
			}

			PlayerPrivate()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~PlayerPrivate()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* aStream) const override
			{
				aStream->WriteUInt(m_level);
				aStream->WriteUInt(m_xp);
				aStream->WritePOD(m_isDead);
				aStream->WriteUInt(m_resurrectionPointMapId);
				m_resurrectionPointPosition.ToStream(aStream);
				m_guildRegistrationHistory.ToStream(aStream);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if(!aStream->ReadUInt(m_level))
					return false;
				if (!aStream->ReadUInt(m_xp))
					return false;
				if (!aStream->ReadPOD(m_isDead))
					return false;
				if (!aStream->ReadUInt(m_resurrectionPointMapId))
					return false;
				if(!m_resurrectionPointPosition.FromStream(aStream))
					return false;
				if(!m_guildRegistrationHistory.FromStream(aStream))
					return false;
				return true;
			}

			// Public data
			uint32_t					m_level = 1;
			uint32_t					m_xp = 0;
			bool						m_isDead = false;
			Vec2						m_resurrectionPointPosition;
			uint32_t					m_resurrectionPointMapId = 0;
			GuildRegistrationHistory	m_guildRegistrationHistory;

			// Not serialized
			bool						m_positionUpdatedOnServer = false;
		};
	}

}