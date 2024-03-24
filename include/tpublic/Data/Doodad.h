#pragma once

#include "../DataBase.h"
#include "../UIntRange.h"
#include "../Vec2.h"

namespace tpublic
{

	namespace Data
	{

		struct Doodad
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_DOODAD;
			static const bool TAGGED = true;

			struct MapPointDefinition
			{
				MapPointDefinition()
				{

				}

				MapPointDefinition(
					const SourceNode*	aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "must_be_one_of")
							aChild->GetIdArray(DataType::ID_SPRITE, m_mustBeOneOf);
						else if(aChild->m_name == "must_not_be_one_of")
							aChild->GetIdArray(DataType::ID_SPRITE, m_mustNotBeOneOf);
						else if(aChild->m_name == "occupy")
							m_occupy = aChild->GetBool();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInts(m_mustBeOneOf);
					aWriter->WriteUInts(m_mustNotBeOneOf);
					aWriter->WriteBool(m_occupy);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if (!aReader->ReadUInts(m_mustBeOneOf))
						return false;
					if (!aReader->ReadUInts(m_mustNotBeOneOf))
						return false;
					if(!aReader->ReadBool(m_occupy))
						return false;
					return true;
				}

				// Public data
				std::vector<uint32_t>	m_mustBeOneOf;
				std::vector<uint32_t>	m_mustNotBeOneOf;
				bool					m_occupy = false;
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
				typedef std::unordered_map<char, size_t> Palette;
				Palette palette;
				
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "sprites")
						{
							aChild->GetIdArray(DataType::ID_SPRITE, m_spriteIds);
						}
						else if (aChild->m_name == "map_palette")
						{
							TP_VERIFY(aChild->m_annotation, aChild->m_debugInfo, "Missing symbol annotation.");
							char symbol = aChild->m_annotation->GetCharacter();
							TP_VERIFY(!palette.contains(symbol), aChild->m_debugInfo, "Symbol '%c' already used.", symbol);
							std::unique_ptr<MapPointDefinition> t = std::make_unique<MapPointDefinition>(aChild);
							palette[symbol] = m_mapPointDefinitions.size();
							m_mapPointDefinitions.push_back(std::move(t));
						}
						else if(aChild->m_name == "map")
						{
							m_mapSize.m_x = (int32_t)strlen(aChild->GetArrayIndex(0)->GetString());
							m_mapSize.m_y = (int32_t)aChild->GetArray()->m_children.size();
							TP_VERIFY(m_mapSize.m_x > 0 && m_mapSize.m_y > 0, aChild->m_debugInfo, "Invalid map.");
							m_map.resize(m_mapSize.m_x * m_mapSize.m_y, 0);
							size_t i = 0;
							for(int32_t y = 0; y < m_mapSize.m_y; y++)
							{
								const char* p = aChild->GetArrayIndex((size_t)y)->GetString();
								TP_VERIFY(strlen(p) == (size_t)m_mapSize.m_x, aChild->m_debugInfo, "Map row length mismatch.");
								for(int32_t x = 0; x < m_mapSize.m_x; x++)
								{
									char symbol = p[x];
									Palette::const_iterator it = palette.find(symbol);
									TP_VERIFY(it != palette.cend(), aChild->m_debugInfo, "Symbol '%c' not defined.", symbol);
									m_map[i] = (uint32_t)it->second;
									i++;
								}
							}
						}
						else if(aChild->m_name == "map_offset")
						{	
							TP_VERIFY(aChild->GetArray()->m_children.size() == 2, aChild->m_debugInfo, "Not a valid offset.");
							m_mapOffset.m_x = aChild->GetArrayIndex(0)->GetInt32();
							m_mapOffset.m_y = aChild->GetArrayIndex(1)->GetInt32();
						}
						else if(aChild->m_name == "auto_probability")
						{
							m_autoProbability = UIntRange(aChild);
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
				IWriter*				aWriter) const override
			{
				aWriter->WriteUInts(m_spriteIds);
				aWriter->WriteObjectPointers(m_mapPointDefinitions);
				m_mapSize.ToStream(aWriter);
				m_mapOffset.ToStream(aWriter);
				aWriter->WriteUInts(m_map);
				aWriter->WriteOptionalObject(m_autoProbability);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadUInts(m_spriteIds))
					return false;
				if(!aReader->ReadObjectPointers(m_mapPointDefinitions))
					return false;
				if (!m_mapSize.FromStream(aReader))
					return false;
				if (!m_mapOffset.FromStream(aReader))
					return false;
				if(!aReader->ReadUInts(m_map))
					return false;
				if(m_map.size() != (size_t)(m_mapSize.m_x * m_mapSize.m_y))
					return false;
				if(!aReader->ReadOptionalObject(m_autoProbability))
					return false;
				return true;
			}

			// Public data
			std::vector<uint32_t>								m_spriteIds;
			std::vector<std::unique_ptr<MapPointDefinition>>	m_mapPointDefinitions;
			Vec2												m_mapSize;
			std::vector<uint32_t>								m_map;
			Vec2												m_mapOffset;
			std::optional<UIntRange>							m_autoProbability;
		};

	}

}