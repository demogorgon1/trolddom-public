#pragma once

#include "../Component.h"
#include "../ComponentBase.h"
#include "../Stat.h"

namespace tpublic
{

	namespace Components
	{

		struct CharacterInfoView
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_CHARACTER_INFO_VIEW;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			enum Field
			{
				FIELD_STATS_FROM_ITEMS,
				FIELD_STATS_FROM_AURAS
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectNoSource<Stat::Collection>(FIELD_STATS_FROM_ITEMS, offsetof(CharacterInfoView, m_statsFromItems));
				aSchema->DefineCustomObjectNoSource<Stat::Collection>(FIELD_STATS_FROM_AURAS, offsetof(CharacterInfoView, m_statsFromAuras));
			}

			// Public data
			Stat::Collection	m_statsFromItems;
			Stat::Collection	m_statsFromAuras;
		};
	}

}