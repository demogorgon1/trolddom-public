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

			PlayerPrivate()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
				, m_guildRegistrationHistory(7 * 24 * 60 * 60) // Count guild registrations within last 7 days
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
			uint32_t			m_level = 1;
			uint32_t			m_xp = 0;
			bool				m_isDead = false;
			Vec2				m_resurrectionPointPosition;
			uint32_t			m_resurrectionPointMapId = 0;
			EventHistory<3>		m_guildRegistrationHistory;

			// Not persistent
			bool				m_positionUpdatedOnServer = false;
		};
	}

}