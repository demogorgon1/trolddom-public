#pragma once

#include "../ComponentBase.h"
#include "../CharacterStat.h"

namespace tpublic
{

	namespace Components
	{

		struct PlayerStats
			: public ComponentBase
		{
		public:
			static const Component::Id ID = Component::ID_PLAYER_STATS;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY | FLAG_REPLICATE_ONLY_ON_REQUEST;
			static const Persistence::Id PERSISTENCE = Persistence::ID_STATS;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			enum Field : uint32_t
			{
				FIELD_PLAYER_STATS
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->DefineCustomObjectNoSource<CharacterStat::Collection>(FIELD_PLAYER_STATS, offsetof(PlayerStats, m_stats));
			}

			template <typename _T>
			void
			Add(
				CharacterStat::Id		aId,
				_T						aValue)
			{
				if(aValue > _T(0))
					m_stats.m_value[aId] += (uint64_t)aValue;
			}

			template <typename _T>
			bool
			UpdateMax(
				CharacterStat::Id		aId,
				_T						aValue)
			{
				if (aValue > _T(0) && (uint64_t)aValue > m_stats.m_value[aId])
				{
					m_stats.m_value[aId] = (uint64_t)aValue;
					return true;
				}
				return false;
			}

			// Public data
			CharacterStat::Collection	m_stats;
		};

	}

}