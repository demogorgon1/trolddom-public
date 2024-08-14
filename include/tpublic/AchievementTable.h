#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	class AchievementTable
	{
	public:
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

		bool
		HasAchievement(
			uint32_t		aAchievementId) const
		{
			return m_map.contains(aAchievementId);
		}

		uint64_t
		GetCompletionTimeStamp(
			uint32_t		aAchievementId) const
		{
			Map::const_iterator i = m_map.find(aAchievementId);
			if (i != m_map.end())
				return i->second->m_completionTimeStamp;
			return 0;
		}

		// Public data 
		typedef std::unordered_map<uint32_t, std::unique_ptr<Entry>> Map;
		Map			m_map;
	};

}