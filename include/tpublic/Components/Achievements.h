#pragma once

#include "../AchievementTable.h"
#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Achievements
			: public ComponentBase
		{
		public:
			static const Component::Id ID = Component::ID_ACHIEVEMENTS;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY | FLAG_REPLICATE_ONLY_ON_REQUEST;
			static const Persistence::Id PERSISTENCE = Persistence::ID_STATS;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Field
			{
				FIELD_TABLE,
				FIELD_TOTAL_POINTS
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->DefineCustomObjectNoSource<AchievementTable>(FIELD_TABLE, offsetof(Achievements, m_table));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_TOTAL_POINTS, NULL, offsetof(Achievements, m_totalPoints));
			}

			bool
			CompleteAchievement(
				uint32_t			aAchievementId,
				uint32_t			aPoints,
				uint64_t			aTimeStamp)
			{
				AchievementTable::Entry* entry = m_table.GetOrCreateEntry(aAchievementId);
				if(entry->m_completionTimeStamp == 0)
				{
					entry->m_completionTimeStamp = aTimeStamp;
					m_totalPoints += aPoints;
					return true;
				}
				return false;
			}

			void
			Reset()
			{
				m_table.m_map.clear();
				m_totalPoints = 0;
			}

			// Public data
			AchievementTable		m_table;
			uint32_t				m_totalPoints = 0;
		};
		
	}

}