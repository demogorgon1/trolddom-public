#pragma once

#include "../Data/Entity.h"

#include "../MapData.h"
#include "../MapGeneratorBase.h"
#include "../TerrainModifier.h"
#include "../UIntRange.h"

namespace tpublic
{
	struct DistanceField;
}

namespace tpublic::MapGenerators
{

	class World
		: public MapGeneratorBase
	{
	public:
		static const MapGenerator::Id ID = MapGenerator::ID_WORLD;

					World();
		virtual		~World();

		enum ExecuteType : uint8_t
		{
			INVALID_EXECUTE_TYPE,

			EXECUTE_TYPE_ALL,
			EXECUTE_TYPE_ONE_OF,
			EXECUTE_TYPE_PALETTED_MAP,
			EXECUTE_TYPE_DOUBLE,
			EXECUTE_TYPE_MEDIAN_FILTER,
			EXECUTE_TYPE_GROW,
			EXECUTE_TYPE_DESPECKLE,
			EXECUTE_TYPE_ADD_BORDERS,
			EXECUTE_TYPE_DEBUG_TERRAIN_MAP,
			EXECUTE_TYPE_OVERLAY,
			EXECUTE_TYPE_TERRAIN_MODIFIER_MAP,
			EXECUTE_TYPE_ADD_ENTITY_SPAWNS,
			EXECUTE_TYPE_ADD_BOSS,
			EXECUTE_TYPE_LEVEL_RANGE,
		};

		struct PalettedMap
		{
			PalettedMap()
			{

			}

			PalettedMap(
				const SourceNode*					aSource)
			{
				m_height = (uint32_t)aSource->GetArray()->m_children.size();
				TP_VERIFY(m_height > 0 && m_height <= 1024, aSource->m_debugInfo, "Invalid paletted map height.");
				m_width = (uint32_t)strlen(aSource->m_children[0]->GetString());
				TP_VERIFY(m_width > 0 && m_width <= 1024, aSource->m_debugInfo, "Invalid paletted map width.");
				m_tiles.resize(m_width * m_height, '\0');

				for(uint32_t y = 0; y < m_height; y++)
				{
					const char* p = aSource->m_children[y]->GetString();
					TP_VERIFY(strlen(p) == (size_t)m_width, aSource->m_debugInfo, "Not all rows are of same length.");

					for (uint32_t x = 0; x < m_width; x++)
					{
						m_tiles[y * m_height + x] = *p;
						p++;
					}
				}
			}

			void
			ToStream(
				IWriter*							aWriter) const
			{
				TP_CHECK(m_tiles.size() > 0 && m_width * m_height == (uint32_t)m_tiles.size(), "Invalid paletted map.");
				aWriter->WriteUInt(m_width);
				aWriter->WriteUInt(m_height);
				aWriter->Write(&m_tiles[0], m_tiles.size());
			}

			bool
			FromStream(
				IReader*							aReader)
			{
				if(!aReader->ReadUInt(m_width))
					return false;
				if(!aReader->ReadUInt(m_height))
					return false;
				if(m_width > 1024 || m_height > 1024 || m_width == 0 || m_height == 0)
					return false;
				m_tiles.resize(m_width * m_height, '\0');
				if(aReader->Read(&m_tiles[0], m_tiles.size()) != m_tiles.size())
					return false;
				return true;
			}

			// Public data
			uint32_t								m_width = 0;
			uint32_t								m_height = 0;
			std::vector<char>						m_tiles;
		};

		struct OverlayTerrainCondition
		{
			enum Type
			{
				INVALID_TYPE,

				TYPE_ABOVE,
				TYPE_BELOW,
				TYPE_RANGE
			};

			OverlayTerrainCondition()
			{

			}

			OverlayTerrainCondition(
				Type								aType,
				const SourceNode*					aSource)
				: m_type(aType)
			{
				if(aSource->m_type == SourceNode::TYPE_ARRAY && aSource->m_children.size() == 2)
				{
					m_value1 = aSource->m_children[0]->GetInt32();
					m_value2 = aSource->m_children[1]->GetInt32();
				}
				else
				{
					m_value1 = aSource->GetInt32();
				}
			}

			void
			ToStream(
				IWriter*							aWriter) const
			{
				aWriter->WritePOD(m_type);
				aWriter->WriteInt(m_value1);
				aWriter->WriteInt(m_value2);
			}

			bool
			FromStream(
				IReader*							aReader)
			{
				if(!aReader->ReadPOD(m_type))
					return false;
				if (!aReader->ReadInt(m_value1))
					return false;
				if (!aReader->ReadInt(m_value2))
					return false;
				return true;
			}

			bool
			Check(
				int32_t								aValue) const
			{
				switch(m_type)
				{
				case TYPE_ABOVE:		return aValue > m_value1;
				case TYPE_BELOW:		return aValue < m_value1;
				case TYPE_RANGE:		return aValue > m_value1 && aValue < m_value2;
				default:				return false;
				}
			}

			// Public data
			Type					m_type;
			int32_t					m_value1 = 0;
			int32_t					m_value2 = 0;
		};

		struct OverlayTerrain
		{
			OverlayTerrain()
			{

			}

			OverlayTerrain(
				const SourceNode*					aSource)
			{
				TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing terrain annotation.");
				m_terrainId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TERRAIN, aSource->m_annotation->GetIdentifier());

				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if (aChild->m_name == "above")
						m_condition = OverlayTerrainCondition(OverlayTerrainCondition::TYPE_ABOVE, aChild);
					else if (aChild->m_name == "below")
						m_condition = OverlayTerrainCondition(OverlayTerrainCondition::TYPE_BELOW, aChild);
					else if (aChild->m_name == "range")
						m_condition = OverlayTerrainCondition(OverlayTerrainCondition::TYPE_RANGE, aChild);
					else if(aChild->m_name == "exclude")
						aChild->GetIdArray(DataType::ID_TERRAIN, m_exclude);
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*							aWriter) const
			{
				aWriter->WriteUInt(m_terrainId);
				m_condition.ToStream(aWriter);
				aWriter->WriteUInts(m_exclude);
			}

			bool
			FromStream(
				IReader*							aReader)
			{
				if (!aReader->ReadUInt(m_terrainId))
					return false;
				if(!m_condition.FromStream(aReader))
					return false;
				if (!aReader->ReadUInts(m_exclude))
					return false;
				return true;
			}

			bool
			IsExcluded(
				uint32_t							aTerrainId) const
			{
				for(uint32_t t : m_exclude)
				{
					if(t == aTerrainId)
						return true;
				}
				return false;
			}

			// Public data
			uint32_t				m_terrainId = 0;
			OverlayTerrainCondition	m_condition;
			std::vector<uint32_t>	m_exclude;
		};

		struct Overlay
		{
			Overlay()
			{

			}

			Overlay(
				const SourceNode*					aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "noise")
						m_noiseId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_NOISE, aChild->GetIdentifier());
					else if(aChild->m_name == "terrain")
						m_terrains.push_back(std::make_unique<OverlayTerrain>(aChild));
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*							aWriter) const
			{
				aWriter->WriteUInt(m_noiseId);
				aWriter->WriteObjectPointers(m_terrains);
			}

			bool
			FromStream(
				IReader*							aReader)
			{
				if(!aReader->ReadUInt(m_noiseId))
					return false;
				if(!aReader->ReadObjectPointers(m_terrains))
					return false;
				return true;
			}

			// Public data
			uint32_t										m_noiseId = 0;
			std::vector<std::unique_ptr<OverlayTerrain>>	m_terrains;
		};

		struct TerrainModifierMap
		{
			TerrainModifierMap()
			{

			}

			TerrainModifierMap(
				const SourceNode*							aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "noise")
					{
						m_noiseId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_NOISE, aChild->GetIdentifier());
					}
					else if(aChild->m_name == "debug")
					{
						m_debug = aChild->GetString();
					}
					else if(aChild->m_name == "terrain_modifier")
					{
						m_id = TerrainModifier::StringToId(aChild->GetIdentifier());
						TP_VERIFY(m_id != TerrainModifier::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid terrain modifier.", aChild->GetIdentifier());
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*								aWriter) const
			{
				aWriter->WritePOD(m_id);
				aWriter->WriteUInt(m_noiseId);
				aWriter->WriteString(m_debug);
			}

			bool
			FromStream(
				IReader*								aReader)
			{
				if (!aReader->ReadPOD(m_id))
					return false;
				if (!aReader->ReadUInt(m_noiseId))
					return false;
				if (!aReader->ReadString(m_debug))
					return false;
				return true;
			}

			// Public data
			TerrainModifier::Id								m_id = TerrainModifier::INVALID_ID;
			uint32_t										m_noiseId = 0;
			std::string										m_debug;
		};

		struct AddEntitySpawns
		{
			struct NeighborTerrain
			{
				NeighborTerrain()
				{

				}

				NeighborTerrain(					
					const SourceNode*						aSource)
				{
					if(aSource->m_name == "neighbor_terrain_probability_bonus")
					{
						TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing terrain annotation.");
						m_terrainId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TERRAIN, aSource->m_annotation->GetIdentifier());
						m_probabilityBonus = aSource->GetUInt32();
					}
					else if(aSource->m_name == "neighbor_terrain_required")
					{
						m_terrainId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TERRAIN, aSource->GetIdentifier());
						m_required = true;
					}
					else
					{
						assert(false);
					}
				}

				void
				ToStream(
					IWriter*								aWriter) const
				{
					aWriter->WriteUInt(m_terrainId);
					aWriter->WriteBool(m_required);
					aWriter->WriteUInt(m_probabilityBonus);
				}

				bool
				FromStream(
					IReader*								aReader) 
				{
					if(!aReader->ReadUInt(m_terrainId))
						return false;
					if(!aReader->ReadBool(m_required))
						return false;
					if(!aReader->ReadUInt(m_probabilityBonus))
						return false;
					return true;
				}

				// Public data
				uint32_t									m_terrainId = 0;
				bool										m_required = false;
				uint32_t									m_probabilityBonus = 0;
			};

			AddEntitySpawns()
			{

			}

			AddEntitySpawns(
				const SourceNode*							aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "entity_spawn")
						aChild->GetIdArray(DataType::ID_MAP_ENTITY_SPAWN, m_mapEntitySpawns);
					else if (aChild->m_name == "probability")
						m_probability = aChild->GetUInt32();
					else if (aChild->m_name == "min_distance_to_nearby")
						m_minDistanceToNearby = aChild->GetUInt32();
					else if (aChild->m_name == "terrain")
						aChild->GetIdArray(DataType::ID_TERRAIN, m_terrains);
					else if(aChild->m_name == "neighbor_terrain_probability_bonus" || aChild->m_name == "neighbor_terrain_required")
						m_neighborTerrains.push_back(NeighborTerrain(aChild));
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*								aWriter) const
			{
				aWriter->WriteUInts(m_mapEntitySpawns);
				aWriter->WriteUInt(m_probability);
				aWriter->WriteUInts(m_terrains);
				aWriter->WriteObjects(m_neighborTerrains);
				aWriter->WriteUInt(m_minDistanceToNearby);
			}

			bool
			FromStream(
				IReader*								aReader)
			{
				if (!aReader->ReadUInts(m_mapEntitySpawns))
					return false;
				if (!aReader->ReadUInt(m_probability))
					return false;
				if (!aReader->ReadUInts(m_terrains))
					return false;
				if (!aReader->ReadObjects(m_neighborTerrains))
					return false;
				if (!aReader->ReadUInt(m_minDistanceToNearby))
					return false;
				return true;
			}

			bool
			CheckTerrain(
				uint32_t								aTerrainId) const
			{
				for(uint32_t t : m_terrains)
				{
					if(t == aTerrainId)
						return true;
				}
				return false;
			}

			// Public data
			std::vector<uint32_t>							m_mapEntitySpawns;
			uint32_t										m_probability = 0;
			std::vector<uint32_t>							m_terrains;			
			std::vector<NeighborTerrain>					m_neighborTerrains;
			uint32_t										m_minDistanceToNearby = 0;
			bool											m_needWalkable = false;
		};

		struct Execute
		{
			Execute()
			{

			}

			Execute(				
				const SourceNode*						aSource)
			{
				TP_VERIFY(aSource->m_tag == "execute", aSource->m_debugInfo, "Invalid execute item.");

				if (aSource->m_name == "all")
					m_type = EXECUTE_TYPE_ALL;
				else if (aSource->m_name == "one_of")
					m_type = EXECUTE_TYPE_ONE_OF;
				else if (aSource->m_name == "paletted_map")
					m_type = EXECUTE_TYPE_PALETTED_MAP;
				else if (aSource->m_name == "double")
					m_type = EXECUTE_TYPE_DOUBLE;
				else if (aSource->m_name == "median_filter")
					m_type = EXECUTE_TYPE_MEDIAN_FILTER;
				else if (aSource->m_name == "grow")
					m_type = EXECUTE_TYPE_GROW;
				else if (aSource->m_name == "despeckle")
					m_type = EXECUTE_TYPE_DESPECKLE;
				else if (aSource->m_name == "add_borders")
					m_type = EXECUTE_TYPE_ADD_BORDERS;
				else if (aSource->m_name == "debug_terrain_map")
					m_type = EXECUTE_TYPE_DEBUG_TERRAIN_MAP;
				else if (aSource->m_name == "overlay")
					m_type = EXECUTE_TYPE_OVERLAY;
				else if (aSource->m_name == "terrain_modifier_map")
					m_type = EXECUTE_TYPE_TERRAIN_MODIFIER_MAP;
				else if (aSource->m_name == "add_entity_spawns")
					m_type = EXECUTE_TYPE_ADD_ENTITY_SPAWNS;
				else if (aSource->m_name == "add_boss")
					m_type = EXECUTE_TYPE_ADD_BOSS;
				else if (aSource->m_name == "level_range")
					m_type = EXECUTE_TYPE_LEVEL_RANGE;

				if(aSource->m_annotation)
					m_weight = aSource->m_annotation->GetUInt32();

				switch(m_type)
				{
				case EXECUTE_TYPE_ALL:
				case EXECUTE_TYPE_ONE_OF:
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						std::unique_ptr<Execute> t = std::make_unique<Execute>(aChild);
						m_totalWeightOfChildren += t->m_weight;
						m_children.push_back(std::move(t));
					});
					break;

				case EXECUTE_TYPE_PALETTED_MAP:
					m_palettedMap = std::make_unique<PalettedMap>(aSource);
					break;

				case EXECUTE_TYPE_TERRAIN_MODIFIER_MAP:
					m_terrainModifierMap = std::make_unique<TerrainModifierMap>(aSource);
					break;

				case EXECUTE_TYPE_OVERLAY:
					m_overlay = std::make_unique<Overlay>(aSource);
					break;

				case EXECUTE_TYPE_MEDIAN_FILTER:
					m_value = aSource->GetUInt32();
					break;

				case EXECUTE_TYPE_ADD_BOSS:
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if (aChild->m_name == "tag_context")
							m_value = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TAG_CONTEXT, aChild->GetIdentifier());
						else if (aChild->m_name == "map_entity_spawn")
							m_value3 = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_ENTITY_SPAWN, aChild->GetIdentifier());
						else if (aChild->m_name == "influence")
							m_value2 = aChild->GetUInt32();
						else 
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
					break;

				case EXECUTE_TYPE_DOUBLE:
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "passes")
							m_value = aChild->GetUInt32();
						else if(aChild->m_name == "no_mutations")
							m_noMutations = aChild->GetBool();
						else 
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
					break;

				case EXECUTE_TYPE_GROW:
				case EXECUTE_TYPE_ADD_BORDERS:
					m_value = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TERRAIN, aSource->GetIdentifier());
					break;

				case EXECUTE_TYPE_DEBUG_TERRAIN_MAP:
					m_string = aSource->GetString();
					break;

				case EXECUTE_TYPE_DESPECKLE:
					break;

				case EXECUTE_TYPE_ADD_ENTITY_SPAWNS:
					m_addEntitySpawns = std::make_unique<AddEntitySpawns>(aSource);
					break;

				case EXECUTE_TYPE_LEVEL_RANGE:
					aSource->GetUIntRange(m_value, m_value2);
					break;

				default:
					TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid execute item.", aSource->m_name.c_str());
					break;
				}
			}

			void
			ToStream(
				IWriter*								aWriter) const
			{
				aWriter->WritePOD(m_type);
				aWriter->WriteUInt(m_weight);
				aWriter->WriteUInt(m_value);
				aWriter->WriteUInt(m_value2);
				aWriter->WriteUInt(m_value3);
				aWriter->WriteBool(m_noMutations);
				aWriter->WriteString(m_string);
				aWriter->WriteOptionalObjectPointer(m_palettedMap);
				aWriter->WriteOptionalObjectPointer(m_overlay);
				aWriter->WriteOptionalObjectPointer(m_terrainModifierMap);
				aWriter->WriteOptionalObjectPointer(m_addEntitySpawns);
				aWriter->WriteObjectPointers(m_children);
				aWriter->WriteUInt(m_totalWeightOfChildren);
			}

			bool
			FromStream(
				IReader*								aReader)
			{
				if(!aReader->ReadPOD(m_type))
					return false;
				if(!aReader->ReadUInt(m_weight))
					return false;
				if (!aReader->ReadUInt(m_value))
					return false;
				if (!aReader->ReadUInt(m_value2))
					return false;
				if (!aReader->ReadUInt(m_value3))
					return false;
				if (!aReader->ReadBool(m_noMutations))
					return false;
				if (!aReader->ReadString(m_string))
					return false;
				if(!aReader->ReadOptionalObjectPointer(m_palettedMap))
					return false;
				if (!aReader->ReadOptionalObjectPointer(m_overlay))
					return false;
				if (!aReader->ReadOptionalObjectPointer(m_terrainModifierMap))
					return false;
				if (!aReader->ReadOptionalObjectPointer(m_addEntitySpawns))
					return false;
				if(!aReader->ReadObjectPointers(m_children))
					return false;
				if(!aReader->ReadUInt(m_totalWeightOfChildren))
					return false;
				return true;
			}

			// Public data
			ExecuteType										m_type = INVALID_EXECUTE_TYPE;
			uint32_t										m_weight = 0;
			uint32_t										m_value = 0;
			uint32_t										m_value2 = 0;
			uint32_t										m_value3 = 0;
			bool											m_noMutations = false;
			std::string										m_string;
			std::unique_ptr<PalettedMap>					m_palettedMap;
			std::unique_ptr<Overlay>						m_overlay;
			std::unique_ptr<TerrainModifierMap>				m_terrainModifierMap;
			std::unique_ptr<AddEntitySpawns>				m_addEntitySpawns;
			std::vector<std::unique_ptr<Execute>>			m_children;
			uint32_t										m_totalWeightOfChildren = 0;
		};
		
		// MapGeneratorBase implementation
		void			FromSource(
							const SourceNode*			aSource) override;
		void			ToStream(
							IWriter*					aWriter) const override;
		bool			FromStream(
							IReader*					aReader) override;
		bool			Build(
							const Manifest*				aManifest,
							MapGeneratorRuntime*		aMapGeneratorRuntime,
							uint32_t					aSeed,
							const MapData*				aSourceMapData,
							const char*					aDebugImagePath,
							std::unique_ptr<MapData>&	aOutMapData) const override;

	private:

		typedef std::unordered_map<char, uint32_t> TerrainPalette;
		TerrainPalette										m_terrainPalette;

		std::vector<std::unique_ptr<Execute>>				m_executes;

		struct Builder
		{
			void			Process(
								const Execute*				aExecute);
			uint32_t		GetTerrainPaletteEntry(
								char						aSymbol) const;
			bool			MaybeDoIt(
								uint32_t					aProbabilty);
			uint32_t		Roll(
								uint32_t					aMin,
								uint32_t					aMax);
			bool			RandomBit();
			uint32_t		Random2Bits();
			uint32_t		GetTerrainIdWithOffset(
								uint32_t					aX,
								uint32_t					aY,
								int32_t						aOffsetX,
								int32_t						aOffsetY) const;
			void			Double(	
								bool						aNoMutations);
			void			MedianFilter(
								uint32_t					aRadius);
			void			Grow(
								uint32_t					aTerrainId);
			void			Despeckle();
			void			AddBorders(
								uint32_t					aTerrainId);
			void			ApplyOverlay(
								const Overlay*				aOverlay);
			void			BuildTerrainModifierMap(
								const TerrainModifierMap*	aTerrainModifierMap);
			void			CreateMapData(
								const MapData*				aSourceMapData,
								std::unique_ptr<MapData>&	aOutMapData);
			void			IdentifyWalkableAreas();
			void			ConnectWalkableAreas();
			bool			TryConnectWalkableArea(
								DistanceField&				aDistanceField);
			size_t			GetWalkableAreaIndexByPosition(	
								const Vec2&					aPosition);
			void			InitNoiseMap();
			uint32_t		SampleNoiseMap(
								const Vec2&					aPosition) const;
			void			GenerateEntitySpawns(
								const AddEntitySpawns*		aAddEntitySpawns);
			void			AddBoss(
								uint32_t					aTagContextId,
								uint32_t					aInfluence,
								uint32_t					aMapEntitySpawnId);
			void			InitBosses();

			// Public data
			const TerrainPalette*							m_terrainPalette = NULL;
			const Manifest*									m_manifest = NULL;
			MapGeneratorRuntime*							m_mapGeneratorRuntime = NULL;
			std::mt19937									m_random;

			uint32_t										m_width = 0;
			uint32_t										m_height = 0;
			std::vector<uint32_t>							m_terrainMap;
			std::vector<int32_t>							m_terrainModifierMaps[TerrainModifier::NUM_IDS];
			uint32_t										m_maxWalkableAreaConnectDistance = 128;			

			struct WalkableArea
			{
				void
				Merge(
					const WalkableArea*						aOther)
				{
					for(const Vec2& position : aOther->m_positions)
						m_positions.insert(position);
				}

				// Public data
				std::unordered_set<Vec2, Vec2::Hasher>		m_positions;
				bool										m_complete = false;
			};

			std::vector<std::unique_ptr<WalkableArea>>		m_walkableAreas;
			std::unordered_set<Vec2, Vec2::Hasher>			m_walkable;

			static const int32_t NOISE_MAP_SIZE = 32;
			uint32_t										m_noiseMap[NOISE_MAP_SIZE * NOISE_MAP_SIZE];			

			std::vector<MapData::EntitySpawn>				m_entitySpawns;
			std::unordered_set<Vec2, Vec2::Hasher>			m_entitySpawnPositions;

			struct Boss
			{
				uint32_t									m_tagContextId = 0;
				uint32_t									m_influence = 32;
				uint32_t									m_mapEntitySpawnId = 0;
				const Data::Entity*							m_entity = NULL;
				std::unique_ptr<DistanceField>				m_distanceField;
				Vec2										m_position;
			};

			struct LevelMapPoint
			{
				const Boss*									m_boss = NULL;
				uint32_t									m_influence = 0;
				uint32_t									m_level = 0;
			};

			std::vector<std::unique_ptr<Boss>>				m_bosses;
			std::unique_ptr<DistanceField>					m_bossDistanceCombined;
			std::vector<LevelMapPoint>						m_levelMap;

			UIntRange										m_levelRange;
		};

		void			_AddTerrainPaletteEntry(
							const SourceNode*			aSource);
	};

}