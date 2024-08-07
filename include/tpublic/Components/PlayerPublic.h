#pragma once

#include "../Component.h"
#include "../ComponentBase.h"
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
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Field
			{
				FIELD_CLASS_ID,
				FIELD_CHARACTER_ID,
				FIELD_STATS
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_CLASS_ID, NULL, offsetof(PlayerPublic, m_classId));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_CHARACTER_ID, NULL, offsetof(PlayerPublic, m_characterId));
				aSchema->DefineCustomObjectNoSource<Stat::Collection>(FIELD_STATS, offsetof(PlayerPublic, m_stats));
			}

			void
			Reset()
			{
				m_classId = 0;
				m_characterId = 0;
				m_stats.Reset();
			}

			// Public data
			uint32_t			m_classId = 0;
			uint32_t			m_characterId = 0;
			Stat::Collection	m_stats;			
		};
	}

}