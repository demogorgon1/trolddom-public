#pragma once

#include "../Component.h"
#include "../LootRule.h"
#include "../PlayerTag.h"
#include "../Rarity.h"

namespace tpublic
{

	namespace Components
	{

		struct Tag
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_TAG;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Field
			{
				FIELD_PLAYER_TAG,
				FIELD_LOOT_RULE,
				FIELD_LOOT_THRESHOLD
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectNoSource<PlayerTag>(FIELD_PLAYER_TAG, offsetof(Tag, m_playerTag));
				aSchema->DefineCustomPODNoSource<LootRule::Id>(FIELD_LOOT_RULE, offsetof(Tag, m_lootRule));
				aSchema->DefineCustomPODNoSource<Rarity::Id>(FIELD_LOOT_THRESHOLD, offsetof(Tag, m_lootThreshold));
			}

			// Public data
			PlayerTag		m_playerTag;
			LootRule::Id	m_lootRule = LootRule::INVALID_ID;
			Rarity::Id		m_lootThreshold = Rarity::INVALID_ID;
		};
	}

}