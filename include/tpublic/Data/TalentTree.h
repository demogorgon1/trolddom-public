#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct TalentTree
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_TALENT_TREE;
			static const bool TAGGED = false;

			struct MapPaletteEntry 
			{
				enum Type : uint8_t
				{
					INVALID_TYPE,

					TYPE_TALENT,
					TYPE_SPRITE
				};

				void
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WritePOD(m_type);
					aStream->WriteUInt(m_id);
					aStream->WritePOD(m_symbol);
				}

				bool
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadPOD(m_type))
						return false;
					if (!aStream->ReadUInt(m_id))
						return false;
					if (!aStream->ReadPOD(m_symbol))
						return false;
					return true;
				}


				// Public data
				Type			m_type = INVALID_TYPE;
				uint32_t		m_id = 0;
				char			m_symbol = '\0';
			};

			struct MapPalette
			{
				MapPalette()
				{

				}

				MapPalette(
					const SourceNode*		aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode*	aChild)
					{
						MapPaletteEntry entry;

						if (aChild->m_tag == "talent")
						{
							entry.m_type = MapPaletteEntry::TYPE_TALENT;
							entry.m_id = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TALENT, aChild->m_name.c_str());
						}
						else if (aChild->m_tag == "sprite")
						{
							entry.m_type = MapPaletteEntry::TYPE_SPRITE;
							entry.m_id = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->m_name.c_str());
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "Not a valid palette entry.");
						}

						const char* symbolString = aChild->GetString();
						TP_VERIFY(strlen(symbolString) == 1, aChild->m_debugInfo, "String must have a length of 1.");
						entry.m_symbol = symbolString[0];

						m_entries.push_back(entry);
					});
				}

				const MapPaletteEntry*
				GetEntry(
					char					aSymbol) const 
				{
					for(const MapPaletteEntry& t : m_entries)
					{
						if(t.m_symbol == aSymbol)
							return &t;
					}
					return NULL;
				}

				void
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteObjects(m_entries);
				}

				bool
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadObjects(m_entries))
						return false;
					return true;
				}

				// Public data
				std::vector<MapPaletteEntry>	m_entries;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			void
			ForEachTalent(
				std::function<void(uint32_t)>	aCallback) const
			{
				if(m_mapPalette)
				{
					for (const MapPaletteEntry& t : m_mapPalette->m_entries)
					{
						if(t.m_type == MapPaletteEntry::TYPE_TALENT)
							aCallback(t.m_id);
					}
				}
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode*	aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "string")
						{
							m_string = aChild->GetString();
						}
						else if (aChild->m_name == "icon")
						{
							m_iconSpriteId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->GetIdentifier());
						}
						else if (aChild->m_name == "map_palette")
						{
							m_mapPalette = std::make_unique<MapPalette>(aChild);
						}
						else if (aChild->m_name == "map")
						{
							aChild->GetArray()->ForEachChild([&](
								const SourceNode* aMapRow)
								{
									m_map.push_back(aMapRow->GetString());
								});
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteString(m_string);
				aStream->WriteUInt(m_iconSpriteId);
				aStream->WriteOptionalObjectPointer(m_mapPalette);
				aStream->WriteStrings(m_map);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadString(m_string))
					return false;
				if(!aStream->ReadUInt(m_iconSpriteId))
					return false;
				if(!aStream->ReadOptionalObjectPointer(m_mapPalette))
					return false;
				if(!aStream->ReadStrings(m_map))
					return false;
				return true;
			}

			// Public data
			std::string					m_string;
			uint32_t					m_iconSpriteId = 0;
			std::unique_ptr<MapPalette>	m_mapPalette;
			std::vector<std::string>	m_map;
		};

	}

}