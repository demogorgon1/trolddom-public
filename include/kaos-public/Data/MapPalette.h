#pragma once

#include "../DataBase.h"

namespace kaos_public
{

	namespace Data
	{

		struct MapPalette
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MAP_PALETTE;

			enum EntryType : uint8_t
			{
				ENTRY_TYPE_TILE,
				ENTRY_TYPE_ENTITY_SPAWN,
				ENTRY_TYPE_PLAYER_SPAWN
			};

			struct Color
			{
				uint8_t			m_r = 0;
				uint8_t			m_g = 0;
				uint8_t			m_b = 0;
			};

			struct Entry
			{
				EntryType		m_type = EntryType(0);
				Color			m_color;
				uint32_t		m_id = 0;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const Parser::Node*		aNode) override
			{
				aNode->GetObject()->ForEachChild([&](
					const Parser::Node*	aChild)
				{
					Entry entry;
					if (aChild->m_tag == "tile")
					{
						entry.m_type = ENTRY_TYPE_TILE;
						entry.m_id = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->m_name.c_str());
					}
					else if (aChild->m_tag == "entity_spawn")
					{
						entry.m_type = ENTRY_TYPE_ENTITY_SPAWN;
						entry.m_id = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_ENTITY_SPAWN, aChild->m_name.c_str());
					}
					else if (aChild->m_tag == "player_spawn")
					{
						entry.m_type = ENTRY_TYPE_PLAYER_SPAWN;
						entry.m_id = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_PLAYER_SPAWN, aChild->m_name.c_str());
					}
					else
					{
						KP_VERIFY(false, aChild->m_debugInfo, "Invalid 'map_palette' item.");
					}

					KP_VERIFY(aChild->m_type == Parser::Node::TYPE_ARRAY, aChild->m_debugInfo, "Not an array.");
					KP_VERIFY(aChild->m_children.size() == 3, aChild->m_debugInfo, "Not a valid RGB color.");

					entry.m_color.m_r = aChild->m_children[0]->GetUInt8();
					entry.m_color.m_g = aChild->m_children[1]->GetUInt8();
					entry.m_color.m_b = aChild->m_children[2]->GetUInt8();

					m_entries.push_back(entry);
				});
			}

			// Public data
			std::vector<Entry>	m_entries;
		};

	}

}