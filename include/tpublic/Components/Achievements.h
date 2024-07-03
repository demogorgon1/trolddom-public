#pragma once

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

			struct Entry
			{
				void
				ToStream(
					IWriter*		aWriter) const
				{
					aWriter->WriteUInt(m_achievementId);
					aWriter->WriteUInt(m_completionTimeStamp);
				}

				bool
				FromStream(
					IReader*		aReader)
				{
					if(!aReader->ReadUInt(m_achievementId))
						return false;
					if(!aReader->ReadUInt(m_completionTimeStamp))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_achievementId = 0;
				uint64_t				m_completionTimeStamp = 0;
			};

			struct Table
			{
				void
				ToStream(
					IWriter*		aWriter) const
				{
					aWriter->WriteUInt(m_map.size());
					for(Map::const_iterator i = m_map.cbegin(); i != m_map.cend(); i++)						
						i->second->ToStream(aWriter);
				}

				bool
				FromStream(
					IReader*		aReader)
				{
					size_t count;
					if(!aReader->ReadUInt(count))
						return false;
					for(size_t i = 0; i < count; i++)
					{
						std::unique_ptr<Entry> t = std::make_unique<Entry>();
						if(!t->FromStream(aReader))
							return false;

						uint32_t achievementId = t->m_achievementId;
						m_map[achievementId] = std::move(t);
					}
					return true;
				}

				Entry*
				GetOrCreateEntry(
					uint32_t		aAchievementId)
				{
					Map::iterator i = m_map.find(aAchievementId);
					if(i != m_map.end())
						return i->second.get();

					Entry* t = new Entry();
					t->m_achievementId = aAchievementId;
					m_map[aAchievementId] = std::unique_ptr<Entry>(t);
					return t;
				}

				// Public data 
				typedef std::unordered_map<uint32_t, std::unique_ptr<Entry>> Map;
				Map			m_map;
			};

			enum Field
			{
				FIELD_TABLE,
				FIELD_TOTAL_POINTS
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->DefineCustomObjectNoSource<Table>(FIELD_TABLE, offsetof(Achievements, m_table));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_TOTAL_POINTS, NULL, offsetof(Achievements, m_totalPoints));
			}

			bool
			HasAchievement(
				uint32_t			aAchievementId) const
			{
				return m_table.m_map.contains(aAchievementId);
			}

			uint64_t
			GetCompletionTimeStamp(
				uint32_t			aAchievementId) const
			{
				Table::Map::const_iterator i = m_table.m_map.find(aAchievementId);
				if (i != m_table.m_map.end())
					return i->second->m_completionTimeStamp;
				return 0;
			}

			void
			CompleteAchievement(
				uint32_t			aAchievementId,
				uint32_t			aPoints,
				uint64_t			aTimeStamp)
			{
				Entry* entry = m_table.GetOrCreateEntry(aAchievementId);
				if(entry->m_completionTimeStamp == 0)
				{
					entry->m_completionTimeStamp = aTimeStamp;
					m_totalPoints += aPoints;
				}
			}

			void
			Reset()
			{
				m_table.m_map.clear();
				m_totalPoints = 0;
			}

			// Public data
			Table					m_table;
			uint32_t				m_totalPoints = 0;
		};
		
	}

}