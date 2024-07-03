#pragma once

#include "../ActionBar.h"
#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct ActionBars
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_ACTION_BARS;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;			
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			static const uint32_t MAX_ROWS = 4;

			enum Field
			{
				FIELD_ACTION_BARS
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectPointersNoSource<ActionBar>(FIELD_ACTION_BARS, offsetof(ActionBars, m_actionBars));
			}

			void
			Set(
				uint32_t	aRow,
				uint32_t	aIndex,
				uint32_t	aAbilityId)
			{
				TP_CHECK(aRow < MAX_ROWS, "Invalid action bar row.");
				TP_CHECK(aIndex < (uint32_t)ActionBar::MAX_SIZE, "Invalid action bar index.");
				while(m_actionBars.size() < (size_t)aRow + 1)
					m_actionBars.push_back(std::make_unique<ActionBar>());
				m_actionBars[aRow]->SetSlot((size_t)aIndex, aAbilityId);
			}

			void
			Reset()
			{
				m_actionBars.clear();
			}
		
			// Public data
			std::vector<std::unique_ptr<ActionBar>>		m_actionBars;
		};
	}

}