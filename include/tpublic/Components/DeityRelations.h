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
						aWriter->WriteUInt(i->second);
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
						std::pair<uint32_t, uint32_t> t;
						if (!aReader->ReadUInt(t.first))
							return false;
						if (!aReader->ReadUInt(t.second))
							return false;

						m_map.insert(t);
					}
					return true;
				}

				// Public data
				typedef std::unordered_map<uint32_t, uint32_t> Map;
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

			uint32_t
			Gain(
				uint32_t			aFactionId,
				uint32_t			aReputation,
				uint32_t			aMaxReputation)
			{
				Table::Map::iterator i = m_table.m_map.find(aFactionId);
				if(i != m_table.m_map.end())
				{					
					uint32_t add = aMaxReputation > i->second ? std::min(aReputation, aMaxReputation - i->second) : 0;
					i->second += add;
					return add;
				}

				uint32_t v = std::min(aReputation, aMaxReputation);
				m_table.m_map[aFactionId] = v;
				return v;
			}

			uint32_t
			Lose(
				uint32_t			aFactionId,
				uint32_t			aReputation)
			{
				Table::Map::iterator i = m_table.m_map.find(aFactionId);
				if(i == m_table.m_map.end())
					return 0;

				uint32_t subtract = std::min(aReputation, i->second);
				i->second -= subtract;
				return subtract;
			}

			// Public data
			Table			m_table;

		};
	}

}