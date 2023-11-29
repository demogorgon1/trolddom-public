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
				FIELD_QUEST_IDS
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectNoSource<RangeBasedSet<uint32_t>>(FIELD_QUEST_IDS, offsetof(CompletedQuests, m_questIds));
			}

			// Public data
			RangeBasedSet<uint32_t>		m_questIds;
		};

	}

}