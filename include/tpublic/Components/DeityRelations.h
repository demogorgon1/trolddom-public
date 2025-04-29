#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct DeityRelations
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_DEITY_RELATIONS;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_NONE;

			struct Entry
			{
				void
				ToStream(
					IWriter*		aWriter) const
				{
					aWriter->WriteUInt(m_desecrations);
					aWriter->WriteUInt(m_prayers);
					aWriter->WriteUInt(m_offerings);
				}

				bool
				FromStream(
					IReader*		aReader)
				{
					if (!aReader->ReadUInt(m_desecrations))
						return false;
					if (!aReader->ReadUInt(m_prayers))
						return false;
					if (!aReader->ReadUInt(m_offerings))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_desecrations = 0;
				uint32_t				m_prayers = 0;
				uint32_t				m_offerings = 0;
			};

			struct Table
			{
				void
				ToStream(
					IWriter*		aWriter) const
				{
					aWriter->WriteUInt(m_map.size());
					for(Map::const_iterator i = m_map.cbegin(); i != m_map.cend(); i++)
					{
						aWriter->WriteUInt(i->first);
						i->second.ToStream(aWriter);
					}
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
						uint32_t deityId;
						if (!aReader->ReadUInt(deityId))
							return false;

						Entry entry;
						if (!entry.FromStream(aReader))
							return false;

						m_map[deityId] = entry;
					}
					return true;
				}

				// Public data
				typedef std::unordered_map<uint32_t, Entry> Map;
				Map			m_map;
			};

			enum Field
			{
				FIELD_TABLE
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->DefineCustomObjectNoSource<Table>(FIELD_TABLE, offsetof(DeityRelations, m_table));
			}
		
			Entry*
			GetEntryForUpdate(
				uint32_t			aDeityId)
			{
				Table::Map::iterator i = m_table.m_map.find(aDeityId);
				if(i != m_table.m_map.end())
					return &i->second;

				return &(m_table.m_map[aDeityId] = Entry());
			}

			const Entry*
			GetEntry(
				uint32_t			aDeityId) const
			{
				Table::Map::const_iterator i = m_table.m_map.find(aDeityId);
				if (i == m_table.m_map.cend())
					return NULL;
				return &i->second;
			}

			void
			Reset()
			{
				m_table.m_map.clear();

				m_deityEvents.clear();
			}

			// Public data
			Table							m_table;

			// Internal, server only
			struct DeityEvent
			{
				uint32_t					m_deityId = 0;
				uint32_t					m_deityPower = 0;
				uint32_t					m_worshipLevel = 0;
				int32_t						m_favor = 0;
				uint32_t					m_itemId = 0;
				bool						m_desecrated = false;
			};

			std::vector<DeityEvent>			m_deityEvents;
		};
	}

}