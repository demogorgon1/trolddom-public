#pragma once

#include "../DataBase.h"

namespace tpublic
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
				ENTRY_TYPE_PLAYER_SPAWN,
				ENTRY_TYPE_PORTAL
			};

			struct Color
			{
				uint8_t			m_r = 0;
				uint8_t			m_g = 0;
				uint8_t			m_b = 0;
			};

			struct Entry
			{
				void
				ToStream(
					IWriter*			aStream) const 
				{
					aStream->WriteUInt(m_type);
					aStream->Write(&m_color, sizeof(m_color));
					aStream->WriteUInt(m_id);
				}

				bool
				FromStream(
					IReader*			aStream) 
				{
					if (!aStream->ReadUInt(m_type))
						return false;
					if(aStream->Read(&m_color, sizeof(m_color)) != sizeof(m_color))
						return false;
					if (!aStream->ReadUInt(m_id))
						return false;
					return true;
				}

				// Public data
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
					else if (aChild->m_tag == "portal")
					{
						entry.m_type = ENTRY_TYPE_PORTAL;
						entry.m_id = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_PORTAL, aChild->m_name.c_str());
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "Invalid 'map_palette' item.");
					}

					TP_VERIFY(aChild->m_type == Parser::Node::TYPE_ARRAY, aChild->m_debugInfo, "Not an array.");
					TP_VERIFY(aChild->m_children.size() == 3, aChild->m_debugInfo, "Not a valid RGB color.");

					entry.m_color.m_r = aChild->m_children[0]->GetUInt8();
					entry.m_color.m_g = aChild->m_children[1]->GetUInt8();
					entry.m_color.m_b = aChild->m_children[2]->GetUInt8();

					m_entries.push_back(entry);
				});
			}

			void
			ToStream(
				IWriter*			aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteObjects(m_entries);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if (!aStream->ReadObjects(m_entries))
					return false;
				return true;
			}

			// Public data
			std::vector<Entry>	m_entries;
		};

	}

}