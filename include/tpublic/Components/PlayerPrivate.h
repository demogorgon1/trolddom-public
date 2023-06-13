#pragma once

#include "../Component.h"

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
				return true;
			}

			// Public data
			uint32_t		m_level = 1;
			uint32_t		m_xp = 0;
			bool			m_isDead = false;

			// Not persistent
			bool			m_positionUpdatedOnServer = false;
		};
	}

}