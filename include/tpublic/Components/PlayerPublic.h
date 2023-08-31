#pragma once

#include "../Component.h"
#include "../Stat.h"
#include "../Vec2.h"

namespace tpublic
{

	namespace Components
	{

		struct PlayerPublic
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_PUBLIC;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			PlayerPublic()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~PlayerPublic()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* aStream) const override
			{
				aStream->WriteUInt(m_classId);
				m_stats.ToStream(aStream);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if(!aStream->ReadUInt(m_classId))
					return false;
				if(!m_stats.FromStream(aStream))
					return false;
				return true;
			}

			void
			DebugPrint() const override
			{
				printf("player_public: class=%u", m_classId);
				m_stats.DebugPrint();
				printf("\n");				
			}

			// Public data
			uint32_t			m_classId = 0;
			Stat::Collection	m_stats;
		};
	}

}