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
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

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
				FIELD_AVAILABLE_POINTS,
				FIELD_HYBRID_REPLACED_TALENT_TREE_ID,
				FIELD_HYBRID_TALENT_TREE_ID,
				FIELD_HYBRID_CLASS_VERSION
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectsNoSource<Entry>(FIELD_ENTRIES, offsetof(Talents, m_entries));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_AVAILABLE_POINTS, NULL, offsetof(Talents, m_availablePoints));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_HYBRID_REPLACED_TALENT_TREE_ID, NULL, offsetof(Talents, m_hybridReplacedTalentTreeId));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_HYBRID_TALENT_TREE_ID, NULL, offsetof(Talents, m_hybridTalentTreeId));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_HYBRID_CLASS_VERSION, NULL, offsetof(Talents, m_hybridClassVersion));
			}

			bool
			IsHybrid() const
			{
				return m_hybridTalentTreeId != 0;
			}

			// The talent trees this character can use: the class' own trees, with the replaced one swapped for the hybrid tree
			void
			GetTalentTreeIds(
				const std::vector<uint32_t>&	aClassTalentTreeIds,
				std::vector<uint32_t>&			aOut) const
			{
				aOut.clear();
				for(uint32_t talentTreeId : aClassTalentTreeIds)
				{
					if(IsHybrid() && talentTreeId == m_hybridReplacedTalentTreeId)
						aOut.push_back(m_hybridTalentTreeId);
					else
						aOut.push_back(talentTreeId);
				}
			}

			bool
			CanUseTalentTree(
				const std::vector<uint32_t>&	aClassTalentTreeIds,
				uint32_t						aTalentTreeId) const
			{
				if(IsHybrid())
				{
					if(aTalentTreeId == m_hybridTalentTreeId)
						return true;
					if(aTalentTreeId == m_hybridReplacedTalentTreeId)
						return false;
				}

				for(uint32_t talentTreeId : aClassTalentTreeIds)
				{
					if(talentTreeId == aTalentTreeId)
						return true;
				}
				return false;
			}

			void
			ClearHybrid()
			{
				m_hybridReplacedTalentTreeId = 0;
				m_hybridTalentTreeId = 0;
				m_hybridClassVersion = 0;
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

			void
			Reset()
			{
				m_entries.clear();
				m_availablePoints = 0;
				ClearHybrid();
			}

			// Public data
			std::vector<Entry>			m_entries;
			uint32_t					m_availablePoints = 0;

			// Hybrid: one of the class' own talent trees replaced by a talent tree from another class. The other class'
			// version is stored so that talents can be reset when it changes.
			uint32_t					m_hybridReplacedTalentTreeId = 0;
			uint32_t					m_hybridTalentTreeId = 0;
			uint32_t					m_hybridClassVersion = 0;
		};
	}

}