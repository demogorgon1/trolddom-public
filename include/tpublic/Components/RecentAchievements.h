#pragma once

#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct RecentAchievements
			: public ComponentBase
		{
		public:
			static const Component::Id ID = Component::ID_RECENT_ACHIEVEMENTS;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_STATS;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			static const size_t MAX_ENTRIES = 5;

			struct Entry
			{
				void
				ToStream(
					IWriter*		aWriter) const
				{
					aWriter->WriteUInt(m_achievementId);
					aWriter->WriteUInt(m_timeStamp);
				}

				bool
				FromStream(
					IReader*		aReader)
				{
					if(!aReader->ReadUInt(m_achievementId))
						return false;
					if(!aReader->ReadUInt(m_timeStamp))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_achievementId = 0;
				uint64_t				m_timeStamp = 0;
			};

			enum Field
			{
				FIELD_ENTRIES,
				FIELD_TOTAL_POINTS
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->DefineCustomObjectsNoSource<Entry>(FIELD_ENTRIES, offsetof(RecentAchievements, m_entries));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_TOTAL_POINTS, NULL, offsetof(RecentAchievements, m_totalPoints));
			}

			void
			Add(
				uint32_t			aAchievementId,
				uint64_t			aTimeStamp)
			{
				if(m_entries.size() < MAX_ENTRIES)
				{
					m_entries.push_back({ aAchievementId, aTimeStamp });
				}
				else
				{
					Entry* oldest = &m_entries[0];
					for(size_t i = 1; i < m_entries.size(); i++)
					{
						if(m_entries[i].m_timeStamp < oldest->m_timeStamp)
							oldest = &m_entries[i];
					}
					oldest->m_achievementId = aAchievementId;
					oldest->m_timeStamp = aTimeStamp;
				}
			}

			void
			Reset()
			{
				m_entries.clear();
				m_totalPoints = 0;
			}

			// Public data
			std::vector<Entry>		m_entries;
			uint32_t				m_totalPoints = 0;
		};

	}

}