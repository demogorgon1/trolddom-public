#pragma once

#include "../DataBase.h"
#include "../WorldInfoMap.h"

namespace tpublic
{

	namespace Data
	{

		struct MapPalette
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MAP_PALETTE;
			static const bool TAGGED = false;

			enum EntryType : uint8_t
			{
				ENTRY_TYPE_TILE,
				ENTRY_TYPE_ENTITY_SPAWN,
				ENTRY_TYPE_PLAYER_SPAWN,
				ENTRY_TYPE_PORTAL,
				ENTRY_TYPE_LEVEL,
				ENTRY_TYPE_ZONE,
				ENTRY_TYPE_WALL,
				ENTRY_TYPE_SUB_ZONE,
				ENTRY_TYPE_ROUTE,
				ENTRY_TYPE_DOODAD,
				ENTRY_TYPE_FLAGS,
				ENTRY_TYPE_CLIFF
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
					aStream->WritePOD(m_type);
					aStream->Write(&m_color, sizeof(m_color));
					aStream->WriteUInt(m_value);
				}

				bool
				FromStream(
					IReader*			aStream) 
				{
					if (!aStream->ReadPOD(m_type))
						return false;
					if(aStream->Read(&m_color, sizeof(m_color)) != sizeof(m_color))
						return false;
					if (!aStream->ReadUInt(m_value))
						return false;
					return true;
				}

				// Public data
				EntryType		m_type = EntryType(0);
				Color			m_color;
				uint32_t		m_value = 0;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{
				aNode->GetObject()->ForEachChild([&](
					const SourceNode*	aChild)
				{
					if(!FromSourceBase(aChild))
					{
						Entry entry;
						if (aChild->m_tag == "tile")
						{
							entry.m_type = ENTRY_TYPE_TILE;
							entry.m_value = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->m_name.c_str());
						}
						else if (aChild->m_tag == "entity_spawn")
						{
							entry.m_type = ENTRY_TYPE_ENTITY_SPAWN;
							entry.m_value = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_ENTITY_SPAWN, aChild->m_name.c_str());
						}
						else if (aChild->m_tag == "player_spawn")
						{
							entry.m_type = ENTRY_TYPE_PLAYER_SPAWN;
							entry.m_value = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_PLAYER_SPAWN, aChild->m_name.c_str());
						}
						else if (aChild->m_tag == "portal")
						{
							entry.m_type = ENTRY_TYPE_PORTAL;
							entry.m_value = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_PORTAL, aChild->m_name.c_str());
						}
						else if (aChild->m_name == "level" && aChild->m_annotation)
						{
							entry.m_type = ENTRY_TYPE_LEVEL;
							entry.m_value = aChild->m_annotation->GetUInt32();
						}
						else if (aChild->m_tag == "zone")
						{
							entry.m_type = ENTRY_TYPE_ZONE;
							entry.m_value = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ZONE, aChild->m_name.c_str());
						}
						else if (aChild->m_tag == "sub_zone")
						{
							entry.m_type = ENTRY_TYPE_SUB_ZONE;
							entry.m_value = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ZONE, aChild->m_name.c_str());
						}
						else if (aChild->m_tag == "wall")
						{
							entry.m_type = ENTRY_TYPE_WALL;
							entry.m_value = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_WALL, aChild->m_name.c_str());
						}
						else if (aChild->m_tag == "route")
						{
							entry.m_type = ENTRY_TYPE_ROUTE;
							entry.m_value = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ROUTE, aChild->m_name.c_str());
						}
						else if (aChild->m_tag == "doodad")
						{
							entry.m_type = ENTRY_TYPE_DOODAD;
							entry.m_value = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_DOODAD, aChild->m_name.c_str());
						}
						else if (aChild->m_name == "flags")
						{
							entry.m_type = ENTRY_TYPE_FLAGS;
							TP_VERIFY(aChild->m_annotation, aChild->m_debugInfo, "Missing flags annotation.");
							entry.m_value = WorldInfoMap::SourceToFlags(aChild->m_annotation.get());
						}
						else if (aChild->m_tag == "cliff")
						{
							entry.m_type = ENTRY_TYPE_CLIFF;
							entry.m_value = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_CLIFF, aChild->m_name.c_str());
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "Invalid 'map_palette' item.");
						}

						TP_VERIFY(aChild->m_type == SourceNode::TYPE_ARRAY, aChild->m_debugInfo, "Not an array.");
						TP_VERIFY(aChild->m_children.size() == 3, aChild->m_debugInfo, "Not a valid RGB color.");

						entry.m_color.m_r = aChild->m_children[0]->GetUInt8();
						entry.m_color.m_g = aChild->m_children[1]->GetUInt8();
						entry.m_color.m_b = aChild->m_children[2]->GetUInt8();

						m_entries.push_back(entry);
					}
				});
			}

			void
			ToStream(
				IWriter*			aStream) const override
			{
				aStream->WriteObjects(m_entries);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if (!aStream->ReadObjects(m_entries))
					return false;
				return true;
			}

			// Public data
			std::vector<Entry>	m_entries;
		};

	}

}