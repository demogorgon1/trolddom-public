#pragma once

#include "../DataBase.h"
#include "../Vec2.h"

namespace tpublic
{

	namespace Data
	{

		struct MapSegment
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MAP_SEGMENT;

			enum Type : uint8_t
			{
				INVALID_TYPE,

				TYPE_PALETTED
			};

			enum ObjectType : uint8_t
			{
				INVALID_OBJECT_TYPE,

				OBJECT_TYPE_PORTAL,
				OBJECT_TYPE_PLAYER_SPAWN,
				OBJECT_TYPE_CONNECTOR
			};

			struct Object
			{
				void
				ToStream(
					IWriter*			aWriter) const
				{
					m_position.ToStream(aWriter);
					aWriter->WritePOD(m_type);
					aWriter->WriteUInt(m_id);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!m_position.FromStream(aReader))
						return false;
					if(!aReader->ReadPOD(m_type))
						return false;
					if(!aReader->ReadUInt(m_id))
						return false;
					return true;
				}

				// Public data
				Vec2				m_position;
				ObjectType			m_type = INVALID_OBJECT_TYPE;
				uint32_t			m_id = 0; 
			};

			struct PaletteEntry
			{	
				PaletteEntry()
				{

				}

				PaletteEntry(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "symbol")
						{							
							const char* symbolString = aChild->GetString();
							TP_VERIFY(symbolString[0] != '\0' && symbolString[1] == '\0', aChild->m_debugInfo, "Not a valid palette entry symbol.");
							m_symbol = (uint32_t)symbolString[0];
						}
						else if(aChild->m_name == "tiles")
						{
							aChild->GetIdArray(DataType::ID_SPRITE, m_tileSpriteIds);
							TP_VERIFY(m_tileSpriteIds.size() > 0, aChild->m_debugInfo, "Must have at least one sprite specified.");
						}
						else if (aChild->m_name == "portals")
						{
							aChild->GetIdArray(DataType::ID_MAP_PORTAL, m_mapPortalIds);
							TP_VERIFY(m_mapPortalIds.size() > 0, aChild->m_debugInfo, "Must have at least one map portal specified.");
						}
						else if (aChild->m_name == "player_spawns")
						{
							aChild->GetIdArray(DataType::ID_MAP_PLAYER_SPAWN, m_mapPlayerSpawnIds);
							TP_VERIFY(m_mapPlayerSpawnIds.size() > 0, aChild->m_debugInfo, "Must have at least one map player spawn specified.");
						}
						else if (aChild->m_name == "connectors")
						{
							aChild->GetIdArray(DataType::ID_MAP_SEGMENT_CONNECTOR, m_mapSegmentConnectorIds);
							TP_VERIFY(m_mapSegmentConnectorIds.size() > 0, aChild->m_debugInfo, "Must have at least one map segment connector specified.");
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					});
				}

				// Public data
				uint32_t				m_symbol = 0;
				std::vector<uint32_t>	m_tileSpriteIds;
				std::vector<uint32_t>	m_mapPortalIds;
				std::vector<uint32_t>	m_mapPlayerSpawnIds;
				std::vector<uint32_t>	m_mapSegmentConnectorIds;
			};
			
			struct Palette
			{
				Palette()
				{

				}

				Palette(
					const SourceNode*	aSource)
				{
					aSource->GetArray()->ForEachChild([&](
						const SourceNode* aChild)
					{
						std::unique_ptr<PaletteEntry> t = std::make_unique<PaletteEntry>(aChild);
						uint32_t symbol = t->m_symbol;
						TP_VERIFY(symbol != ' ', aChild->m_debugInfo, "Empty space symbol is reserved.");
						TP_VERIFY(!m_table.contains(symbol), aChild->m_debugInfo, "Palette entry symbol already in use.");
						m_table[symbol] = std::move(t);
					});
				}

				const PaletteEntry*
				GetEntry(
					uint32_t			aSymbol) const
				{
					Table::const_iterator i = m_table.find(aSymbol);
					if(i == m_table.cend())
						return NULL;
					return i->second.get();
				}

				// Public data
				typedef std::unordered_map<uint32_t, std::unique_ptr<PaletteEntry>> Table;
				Table					m_table;
			};

			struct TileMap
			{
				TileMap()
				{

				}
			
				TileMap(					
					const Palette*		aPalette,
					const SourceNode*	aSource,
					std::mt19937&		aRandom)
				{
					TP_VERIFY(aPalette != NULL, aSource->m_debugInfo, "Palette has to be defined first.");
					TP_VERIFY(aSource->m_type == SourceNode::TYPE_ARRAY, aSource->m_debugInfo, "Not a valid paletted map.");
					
					m_size.m_y = (int32_t)aSource->m_children.size();
					TP_VERIFY(m_size.m_y > 0, aSource->m_debugInfo, "Must have at least one row.");
					
					size_t offset = 0;

					for(int32_t y = 0; y < m_size.m_y; y++)
					{
						const SourceNode* child = aSource->m_children[y].get();
						const char* row = child->GetString();
						size_t rowLength = strlen(row);

						if(m_size.m_x == 0)
						{
							TP_VERIFY(rowLength > 0, child->m_debugInfo, "Must have at least one column.");
							m_size.m_x = (int32_t)rowLength;
							m_tiles.resize(m_size.m_x * m_size.m_y, 0);
						}
						else
						{
							TP_VERIFY(m_size.m_x == (int32_t)rowLength, child->m_debugInfo, "Row length mismatch.");
						}

						for(int32_t x = 0; x < m_size.m_x; x++)
						{
							uint32_t symbol = (uint32_t)row[x];
							assert(offset < m_tiles.size());

							if(symbol != ' ')
							{
								const PaletteEntry* paletteEntry = aPalette->GetEntry(symbol);
								TP_VERIFY(paletteEntry != NULL, child->m_debugInfo, "'%c' is not defined in palette.", symbol);

								if(paletteEntry->m_tileSpriteIds.size() > 1)
								{
									std::uniform_int_distribution<size_t> distribution(0, paletteEntry->m_tileSpriteIds.size() - 1);
									m_tiles[offset] = paletteEntry->m_tileSpriteIds[distribution(aRandom)];
								}
								else if(paletteEntry->m_tileSpriteIds.size() == 1)
								{
									m_tiles[offset] = paletteEntry->m_tileSpriteIds[0];
								}			
								
								if(paletteEntry->m_mapPlayerSpawnIds.size() > 0 )
								{
									Object t;
									t.m_position = { x, y };
									t.m_type = OBJECT_TYPE_PLAYER_SPAWN;
									
									if(paletteEntry->m_mapPlayerSpawnIds.size() == 1)
									{
										t.m_id = paletteEntry->m_mapPlayerSpawnIds[0];
									}
									else
									{
										std::uniform_int_distribution<size_t> distribution(0, paletteEntry->m_mapPlayerSpawnIds.size() - 1);
										t.m_id = paletteEntry->m_mapPlayerSpawnIds[distribution(aRandom)];
									}

									m_objects.push_back(t);
								}

								if (paletteEntry->m_mapPortalIds.size() > 0)
								{
									Object t;
									t.m_position = { x, y };
									t.m_type = OBJECT_TYPE_PORTAL;

									if (paletteEntry->m_mapPortalIds.size() == 1)
									{
										t.m_id = paletteEntry->m_mapPortalIds[0];
									}
									else
									{
										std::uniform_int_distribution<size_t> distribution(0, paletteEntry->m_mapPortalIds.size() - 1);
										t.m_id = paletteEntry->m_mapPortalIds[distribution(aRandom)];
									}

									m_objects.push_back(t);
								}

								if (paletteEntry->m_mapSegmentConnectorIds.size() > 0)
								{
									Object t;
									t.m_position = { x, y };
									t.m_type = OBJECT_TYPE_CONNECTOR;

									if (paletteEntry->m_mapSegmentConnectorIds.size() == 1)
									{
										t.m_id = paletteEntry->m_mapSegmentConnectorIds[0];
									}
									else
									{
										std::uniform_int_distribution<size_t> distribution(0, paletteEntry->m_mapSegmentConnectorIds.size() - 1);
										t.m_id = paletteEntry->m_mapSegmentConnectorIds[distribution(aRandom)];
									}

									m_objects.push_back(t);
								}
							}

							offset++;
						}
					}
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					m_size.ToStream(aWriter);
					aWriter->WriteUInts(m_tiles);
					aWriter->WriteObjects(m_objects);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!m_size.FromStream(aReader))
						return false;
					if(!aReader->ReadUInts(m_tiles))
						return false;
					if(m_tiles.size() != (size_t)(m_size.m_x * m_size.m_y))
						return false;
					if(!aReader->ReadObjects(m_objects))
						return false;
					return true;
				}

				// Public data
				Vec2					m_size;
				std::vector<uint32_t>	m_tiles;
				std::vector<Object>		m_objects;
			};

			struct TileMapModifier
			{
				enum Modifier : uint8_t
				{
					INVALID_MODIFIER,

					MODIFIER_WALL_FACE
				};

				TileMapModifier()
				{

				}

				TileMapModifier(
					const SourceNode*	aSource)
				{
					if(aSource->m_name == "wall_face")
						m_modifier = MODIFIER_WALL_FACE;

					TP_VERIFY(m_modifier != INVALID_MODIFIER, aSource->m_debugInfo, "'%s' is not a valid modifier.", aSource->m_name.c_str());

					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if (aChild->m_name == "walls" && m_modifier == MODIFIER_WALL_FACE)
							aChild->GetIdArray(DataType::ID_SPRITE, m_wallSpriteIds);
						else if (aChild->m_name == "floors" && m_modifier == MODIFIER_WALL_FACE)
							aChild->GetIdArray(DataType::ID_SPRITE, m_floorSpriteIds);
						else if (aChild->m_name == "faces" && m_modifier == MODIFIER_WALL_FACE)
							aChild->GetIdArray(DataType::ID_SPRITE, m_faceSpriteIds);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{				
					aWriter->WritePOD(m_modifier);

					if(m_modifier == MODIFIER_WALL_FACE)
					{
						aWriter->WriteUInts(m_wallSpriteIds);
						aWriter->WriteUInts(m_floorSpriteIds);
						aWriter->WriteUInts(m_faceSpriteIds);
					}
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadPOD(m_modifier))
						return false;

					if(m_modifier == MODIFIER_WALL_FACE)
					{
						if (!aReader->ReadUInts(m_wallSpriteIds))
							return false;
						if (!aReader->ReadUInts(m_floorSpriteIds))
							return false;
						if (!aReader->ReadUInts(m_faceSpriteIds))
							return false;
					}
					return true;
				}

				void
				Apply(
					TileMap*			aTileMap,
					std::mt19937&		aRandom) const
				{
					switch(m_modifier)
					{
					case MODIFIER_WALL_FACE:
						{
							int32_t i = 0;
							for (int32_t y = 0; y < aTileMap->m_size.m_y - 1; y++)
							{
								for (int32_t x = 0; x < aTileMap->m_size.m_x; x++)
								{
									assert((size_t)i < aTileMap->m_tiles.size());
									uint32_t& spriteId = aTileMap->m_tiles[i];
									const uint32_t& southSpriteId = aTileMap->m_tiles[i + aTileMap->m_size.m_x];

									if(std::find(m_wallSpriteIds.cbegin(), m_wallSpriteIds.cend(), spriteId) != m_wallSpriteIds.cend() && 
										std::find(m_floorSpriteIds.cbegin(), m_floorSpriteIds.cend(), southSpriteId) != m_floorSpriteIds.cend())
									{
										if(m_faceSpriteIds.size() == 1)
										{
											spriteId = m_faceSpriteIds[0];
										}
										else
										{
											TP_CHECK(m_faceSpriteIds.size() > 1);
											std::uniform_int_distribution<size_t> distribution(0, m_faceSpriteIds.size() - 1);
											spriteId = m_faceSpriteIds[distribution(aRandom)];
										}
									}

									i++;
								}
							}
						}
						break;

					default:
						TP_CHECK(false, "Invalid tile map modifier.");
						break;
					}
				}

				// Public data
				Modifier				m_modifier = INVALID_MODIFIER;

				// INVALID_MODIFIER
				std::vector<uint32_t>	m_wallSpriteIds;
				std::vector<uint32_t>	m_floorSpriteIds;
				std::vector<uint32_t>	m_faceSpriteIds;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{	
				TP_VERIFY(aSource->m_annotation && aSource->m_annotation->m_type == SourceNode::TYPE_IDENTIFIER, aSource->m_debugInfo, "Missing type annotation.");
				const char* typeString = aSource->m_annotation->GetIdentifier();

				if(strcmp(typeString, "paletted") == 0)
					m_type = TYPE_PALETTED;

				TP_VERIFY(m_type != INVALID_TYPE, aSource->m_debugInfo, "'%s' is not a valid type.", typeString);

				std::unique_ptr<Palette> palette;
				std::mt19937 random;

				aSource->ForEachChild([&](
					const SourceNode*	aChild)
				{
					if(aChild->m_name == "palette" && m_type == TYPE_PALETTED)
						palette = std::make_unique<Palette>(aChild);
					else if(aChild->m_name == "paletted_map" && m_type == TYPE_PALETTED)
						m_tileMap = std::make_unique<TileMap>(palette.get(), aChild, random);
					else if(aChild->m_tag == "tile_map_modifier")
						m_tileMapModifiers.push_back(std::make_unique<TileMapModifier>(aChild));
					else 
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});			
				
				if(m_tileMap && m_tileMapModifiers.size() > 0)
				{
					for(const std::unique_ptr<TileMapModifier>& tileMapModifier : m_tileMapModifiers)
						tileMapModifier->Apply(m_tileMap.get(), random);
				}
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);

				aStream->WritePOD(m_type);
				aStream->WriteOptionalObjectPointer(m_tileMap);
				aStream->WriteObjectPointers(m_tileMapModifiers);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;

				if (!aStream->ReadPOD(m_type))
					return false;
				if (!aStream->ReadOptionalObjectPointer(m_tileMap))
					return false;
				if(!aStream->ReadObjectPointers(m_tileMapModifiers))
					return false;
				return true;
			}

			// Public data
			Type											m_type = INVALID_TYPE;
			std::unique_ptr<TileMap>						m_tileMap;
			std::vector<std::unique_ptr<TileMapModifier>>	m_tileMapModifiers;
		};

	}

}