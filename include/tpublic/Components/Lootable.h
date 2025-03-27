#pragma once

#include "../Component.h"
#include "../ComponentBase.h"
#include "../ComponentSchema.h"
#include "../ItemInstance.h"
#include "../PlayerTag.h"

namespace tpublic
{

	namespace Components
	{

		struct Lootable
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_LOOTABLE;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			struct AvailableLoot
			{
				void
				ToStream(
					IWriter*			aWriter) const
				{
					m_playerTag.ToStream(aWriter);
					m_itemInstance.ToStream(aWriter);
					aWriter->WriteUInt(m_questId);
					aWriter->WriteUInt(m_lootCooldownId);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!m_playerTag.FromStream(aReader))
						return false;
					if (!m_itemInstance.FromStream(aReader))
						return false;
					if(!aReader->ReadUInt(m_questId))
						return false;
					if (!aReader->ReadUInt(m_lootCooldownId))
						return false;
					return true;
				}

				// Public data
				PlayerTag				m_playerTag;
				ItemInstance			m_itemInstance;
				uint32_t				m_questId = 0;
				uint32_t				m_lootCooldownId = 0;
			};

			enum Field
			{
				FIELD_LOOT_TABLE_ID,
				FIELD_PLAYER_TAG,
				FIELD_CASH,
				FIELD_AVAILABLE_CASH,
				FIELD_AVAILABLE_LOOT,
				FIELD_VERSION,
				FIELD_TIME_STAMP,
				FIELD_ANYONE_CAN_LOOT,
				FIELD_SPECIAL_LOOT_COOLDOWN_ID
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_LOOT_TABLE_ID, "loot_table", offsetof(Lootable, m_lootTableId))->SetDataType(DataType::ID_LOOT_TABLE);
				aSchema->DefineCustomObjectNoSource<PlayerTag>(FIELD_PLAYER_TAG, offsetof(Lootable, m_playerTag));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_CASH, NULL, offsetof(Lootable, m_cash));
				aSchema->Define(ComponentSchema::TYPE_INT64, FIELD_AVAILABLE_CASH, NULL, offsetof(Lootable, m_availableCash));
				aSchema->DefineCustomObjectsNoSource<AvailableLoot>(FIELD_AVAILABLE_LOOT, offsetof(Lootable, m_availableLoot));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_VERSION, NULL, offsetof(Lootable, m_version));
				aSchema->Define(ComponentSchema::TYPE_UINT64, FIELD_TIME_STAMP, NULL, offsetof(Lootable, m_timeStamp));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_ANYONE_CAN_LOOT, "anyone_can_loot", offsetof(Lootable, m_anyoneCanLoot));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_SPECIAL_LOOT_COOLDOWN_ID, "special_loot_cooldown", offsetof(Lootable, m_specialLootCooldownId))->SetDataType(DataType::ID_LOOT_COOLDOWN);
			}

			bool
			GetAvailableLootByIndex(
				size_t					aIndex,
				AvailableLoot&			aOut) const
			{
				if(aIndex >= m_availableLoot.size())
					return false;
				aOut = m_availableLoot[aIndex];
				return true;
			}

			void
			SetPlayerTagForAllAvailableLoot(
				const PlayerTag&		aPlayerTag)
			{
				for(AvailableLoot& t : m_availableLoot)
				{
					if(!t.m_playerTag.IsSet())
						t.m_playerTag = aPlayerTag;
				}
			}

			bool
			HasLoot() const
			{
				if(m_cash)
					return true;

				for(const AvailableLoot& t : m_availableLoot)
				{
					if(t.m_itemInstance.IsSet())
						return true;
				}

				return false;
			}

			void
			Reset()
			{
				m_lootTableId = 0;
				m_playerTag.Clear();
				m_cash = false;
				m_availableCash = 0;
				m_availableLoot.clear();
				m_timeStamp = 0;
				m_version = 0;
				m_anyoneCanLoot = false;
				m_specialLootCooldownId = 0;
			}

			// Public data
			uint32_t					m_lootTableId = 0;
			bool						m_anyoneCanLoot = false;
			PlayerTag					m_playerTag;
			bool						m_cash = false;
			int64_t						m_availableCash = 0;
			std::vector<AvailableLoot>	m_availableLoot;
			uint64_t					m_timeStamp = 0;
			uint32_t					m_version = 0;	
			uint32_t					m_specialLootCooldownId = 0;
		};
	}

}