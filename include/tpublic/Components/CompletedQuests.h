#pragma once

#include "../ComponentBase.h"
#include "../RangeBasedSet.h"

namespace tpublic
{

	namespace Components
	{

		struct CompletedQuests
			: public ComponentBase
		{
		public:
			static const Component::Id ID = Component::ID_COMPLETED_QUESTS;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			enum Field
			{
				FIELD_QUEST_IDS,
				FIELD_DAILY_QUEST_IDS,
				FIELD_DAILY_RESET_TIME_STAMP
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectNoSource<RangeBasedSet<uint32_t>>(FIELD_QUEST_IDS, offsetof(CompletedQuests, m_questIds));
				aSchema->DefineCustomObjectNoSource<RangeBasedSet<uint32_t>>(FIELD_DAILY_QUEST_IDS, offsetof(CompletedQuests, m_dailyQuestIds));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_DAILY_RESET_TIME_STAMP, NULL, offsetof(CompletedQuests, m_dailyResetTimeStamp));
			}

			void
			Reset()
			{
				m_questIds.Reset();
				m_dailyQuestIds.Reset();
				m_dailyResetTimeStamp = 0;
			}

			bool
			HasQuest(
				uint32_t		aQuestId) const
			{
				return m_questIds.HasValue(aQuestId) || m_dailyQuestIds.HasValue(aQuestId);
			}

			// Public data
			RangeBasedSet<uint32_t>		m_questIds;
			RangeBasedSet<uint32_t>		m_dailyQuestIds;
			uint32_t					m_dailyResetTimeStamp = 0;
		};

	}

}