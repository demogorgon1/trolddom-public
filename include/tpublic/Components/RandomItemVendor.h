#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct RandomItemVendor
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_RANDOM_ITEM_VENDOR;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			struct Item
			{
				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_itemId);
					aWriter->WriteInt(m_tick);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_itemId))
						return false;
					if(!aReader->ReadInt(m_tick))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_itemId = 0;
				int32_t				m_tick = 0;
			};

			struct TokenCost
			{
				void 
				FromSource(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "token_item")
							m_tokenItemId = aChild->GetId(DataType::ID_ITEM);
						else if (aChild->m_name == "token_sprite")
							m_tokenSpriteId = aChild->GetId(DataType::ID_SPRITE);
						else if(aChild->m_name == "conversion")
							m_conversion = aChild->GetUInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_tokenItemId);
					aWriter->WriteUInt(m_tokenSpriteId);
					aWriter->WriteUInt(m_conversion);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if (!aReader->ReadUInt(m_tokenItemId))
						return false;
					if (!aReader->ReadUInt(m_tokenSpriteId))
						return false;
					if (!aReader->ReadUInt(m_conversion))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_tokenItemId = 0;
				uint32_t			m_tokenSpriteId = 0;
				uint32_t			m_conversion = 0;
			};

			enum Field
			{
				FIELD_ITEM_COUNT,
				FIELD_LOOT_TABLE_ID,
				FIELD_LEVEL,
				FIELD_REPLACEMENT_TICKS,
				FIELD_REFRESH_TICKS,
				FIELD_TOKEN_COST,
				FIELD_ITEMS,
				FIELD_VERSION,
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_ITEM_COUNT, "item_count", offsetof(RandomItemVendor, m_itemCount));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_LOOT_TABLE_ID, "loot_table", offsetof(RandomItemVendor, m_lootTableId))->SetDataType(DataType::ID_LOOT_TABLE);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_LEVEL, "level", offsetof(RandomItemVendor, m_level));
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_REPLACEMENT_TICKS, "replacement_ticks", offsetof(RandomItemVendor, m_replacementTicks));
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_REFRESH_TICKS, "refresh_ticks", offsetof(RandomItemVendor, m_refreshTicks));
				aSchema->DefineCustomOptionalObject<TokenCost>(FIELD_TOKEN_COST, "token_cost", offsetof(RandomItemVendor, m_tokenCost));
				aSchema->DefineCustomObjectsNoSource<Item>(FIELD_ITEMS, offsetof(RandomItemVendor, m_items));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_VERSION, NULL, offsetof(RandomItemVendor, m_version));
			}

			void
			Reset()
			{
				m_itemCount = 0;
				m_lootTableId = 0;
				m_level = 0;
				m_replacementTicks = 0;
				m_refreshTicks = 0;
				m_tokenCost.reset();
				m_items.clear();
				m_version = 0;
			}

			bool
			HasItem(
				uint32_t				aItemId) const
			{
				for(const Item& t : m_items)
				{
					if(t.m_itemId == aItemId)
						return true;
				}
				return false;
			}

			bool
			RemoveItem(
				uint32_t				aItemId)
			{
				for (Item& t : m_items)
				{
					if (t.m_itemId == aItemId)
					{
						t.m_itemId = 0;
						t.m_tick = 0;
						m_version++;
						return true;
					}
				}
				return false;

			}

			// Public data
			uint32_t					m_itemCount = 0;
			uint32_t					m_lootTableId = 0;
			uint32_t					m_level = 0;
			int32_t						m_replacementTicks = 0;
			int32_t						m_refreshTicks = 0;
			std::optional<TokenCost>	m_tokenCost;
			std::vector<Item>			m_items;
			uint32_t					m_version = 0;
		};
	}

}