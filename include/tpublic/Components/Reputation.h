#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Reputation
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_REPUTATION;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

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
						aWriter->WriteInt(i->second);
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
						uint32_t factionId;
						if (!aReader->ReadUInt(factionId))
							return false;

						int32_t reputation;
						if (!aReader->ReadInt(reputation))
							return false;

						m_map[factionId] = reputation;
					}
					return true;
				}

				// Public data
				typedef std::unordered_map<uint32_t, int32_t> Map;
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
				aSchema->DefineCustomObjectNoSource<Table>(FIELD_TABLE, offsetof(Reputation, m_table));
			}

			int32_t
			Modify(
				uint32_t			aFactionId,
				int32_t				aReputation,
				int32_t				aMinReputation,
				int32_t				aMaxReputation,
				bool&				aOutReputationWasPositive)
			{
				assert(aFactionId != 0);
				Table::Map::iterator i = m_table.m_map.find(aFactionId);
				if(i != m_table.m_map.end())
				{					
					int32_t oldValue = i->second;
					int32_t newValue = i->second + aReputation;
					if(newValue < aMinReputation)
						newValue = aMinReputation;
					if (newValue > aMaxReputation)
						newValue = aMaxReputation;
					i->second = newValue;
					aOutReputationWasPositive = oldValue > 0;
					m_wasUpdated = true;
					return newValue - oldValue;
				}

				int32_t newValue = aReputation;
				if (newValue < aMinReputation)
					newValue = aMinReputation;
				if (newValue > aMaxReputation)
					newValue = aMaxReputation;
				aOutReputationWasPositive = false;
				m_table.m_map[aFactionId] = newValue;
				m_wasUpdated = true;
				return newValue;
			}

			int32_t
			GetReputation(
				uint32_t			aFactionId) const
			{
				Table::Map::const_iterator i = m_table.m_map.find(aFactionId);
				if(i == m_table.m_map.cend())
					return 0;

				return i->second;
			}

			void
			Reset()
			{
				m_table.m_map.clear();
			}

			// Public data
			Table			m_table;

			// Internal
			bool			m_wasUpdated = false;
		};
	}

}