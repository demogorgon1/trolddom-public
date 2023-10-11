#pragma once

#include "../Component.h"

namespace tpublic
{

	namespace Components
	{

		struct Talents
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_TALENTS;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;

			struct Entry
			{
				void
				ToStream(
					IWriter*	aStream) const
				{
					aStream->WriteUInt(m_talentId);
					aStream->WriteUInt(m_points);
				}

				bool
				FromStream( 
					IReader*	aStream)
				{
					if(!aStream->ReadUInt(m_talentId))
						return false;
					if(!aStream->ReadUInt(m_points))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_talentId = 0;
				uint32_t				m_points = 0;
			};

			enum Field
			{
				FIELD_ENTRIES,
				FIELD_AVAILABLE_POINTS
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectsNoSource<Entry>(FIELD_ENTRIES, offsetof(Talents, m_entries));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_AVAILABLE_POINTS, NULL, offsetof(Talents, m_availablePoints));
			}

			bool
			HasTalent(
				uint32_t		aTalentId) const
			{
				for(const Entry& t : m_entries)
				{
					if(t.m_talentId == aTalentId)
						return true;
				}
				return false;
			}

			uint32_t
			GetPoints(
				uint32_t		aTalentId) const
			{
				for (const Entry& t : m_entries)
				{
					if (t.m_talentId == aTalentId)
						return t.m_points;
				}
				return 0;
			}

			bool
			AddPoint(
				uint32_t		aTalentId,
				uint32_t		aMaxPoints) 
			{
				if(m_availablePoints == 0)
					return false;

				for (Entry& existing : m_entries)
				{
					if(existing.m_talentId == aTalentId)
					{
						if(existing.m_points < aMaxPoints)
						{
							existing.m_points++;
							m_availablePoints--;
							return true;
						}
						else
						{
							return false;
						}
					}
				}

				m_availablePoints--;
				m_entries.push_back({ aTalentId, 1 });
				return true;
			}

			bool
			ResetPoints()
			{
				if(m_entries.size() == 0)
					return false;

				for (Entry& t : m_entries)
					m_availablePoints += t.m_points;

				m_entries.clear();

				return true;
			}

			// Public data
			std::vector<Entry>			m_entries;
			uint32_t					m_availablePoints = 0;
		};
	}

}