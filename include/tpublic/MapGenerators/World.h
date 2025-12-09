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

		struct IExecuteFactory;
		typedef std::unordered_map<char, uint32_t> TerrainPalette;

		struct Params
		{
			void
			ToStream(
				IWriter*									aWriter) const
			{
				aWriter->WriteUInt(m_playerSpawnEntityId);
				aWriter->WriteUInt(m_objectMapEntitySpawnId);
				aWriter->WriteUInts(m_connectConversionRadius);
				aWriter->WriteUInt(m_playerSpawnZoneRadius);
				aWriter->WriteUInt(m_playerSpawnZoneId);
				aWriter->WriteUInt(m_outsidePlayerSpawnZoneId);
			}

			bool
			FromStream(
				IReader*									aReader)
			{
				if (!aReader->ReadUInt(m_playerSpawnEntityId))
					return false;
				if (!aReader->ReadUInt(m_objectMapEntitySpawnId))
					return false;
				if (!aReader->ReadUInts(m_connectConversionRadius))
					return false;
				if (!aReader->ReadUInt(m_playerSpawnZoneRadius))
					return false;
				if (!aReader->ReadUInt(m_playerSpawnZoneId))
					return false;
				if (!aReader->ReadUInt(m_outsidePlayerSpawnZoneId))
					return false;
				return true;
			}

			// Public data
			uint32_t					m_playerSpawnEntityId = 0;
			uint32_t					m_objectMapEntitySpawnId = 0;
			std::vector<uint32_t>		m_connectConversionRadius;
			uint32_t					m_playerSpawnZoneRadius = 0;
			uint32_t					m_playerSpawnZoneId = 0;
			uint32_t					m_outsidePlayerSpawnZoneId = 0;
		};

		struct MinorBosses
		{
			MinorBosses()
			{

			}

			MinorBosses(
				const SourceNode*							aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "count")
						m_count = UIntRange(aChild);
					else if (aChild->m_name == "map_entity_spawn")
						m_mapEntitySpawnId = aChild->GetId(DataType::ID_MAP_ENTITY_SPAWN);
					else if (aChild->m_name == "tag_context")
						m_tagContextId = aChild->GetId(DataType::ID_TAG_CONTEXT);
					else if (aChild->m_name == "influence_range")
						m_influenceRange = UIntRange(aChild);
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*								aWriter) const
			{
				aWriter->WriteUInt(m_tagContextId);
				m_count.ToStream(aWriter);
				aWriter->WriteUInt(m_mapEntitySpawnId);
				m_influenceRange.ToStream(aWriter);
			}

			bool
			FromStream(
				IReader*								aReader)
			{
				if (!aReader->ReadUInt(m_tagContextId))
					return false;
				if(!m_count.FromStream(aReader))
					return false;
				if (!aReader->ReadUInt(m_mapEntitySpawnId))
					return false;
				if (!m_influenceRange.FromStream(aReader))
					return false;
				return true;
			}

			// Public data
			uint32_t								m_tagContextId = 0;
			UIntRange								m_count;
			uint32_t								m_mapEntitySpawnId = 0;
			UIntRange								m_influenceRange;
		};

		struct RandomObject
		{
			RandomObject()
			{

			}

			RandomObject(
				const SourceNode*							aSource)
			{
				TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing probability annotation.");
				m_probability = aSource->m_annotation->GetUInt32();
				m_mapEntitySpawnId = aSource->GetId(DataType::ID_MAP_ENTITY_SPAWN);
			}

			void
			ToStream(
				IWriter*									aWriter) const
			{
				aWriter->WriteUInt(m_probability);
				aWriter->WriteUInt(m_mapEntitySpawnId);
			}

			bool
			FromStream(
				IReader*									aReader) 
			{
				if(!aReader->ReadUInt(m_probability))
					return false;
				if(!aReader->ReadUInt(m_mapEntitySpawnId))
					return false;
				return true;
			}

			// Public data
			uint32_t								m_probability = 0;
			uint32_t								m_mapEntitySpawnId = 0;
		};

		struct Pack
		{
			Pack()
			{

			}

			Pack(
				const SourceNode*							aSource)
			{
				TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing probability annotation.");
				m_probability = aSource->m_annotation->GetUInt32();

				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if (aChild->m_name == "min_distance_to_nearby")
						m_minDistanceToNearBy = aChild->GetUInt32();
					else if (aChild->m_name == "elite")
						m_elite = aChild->GetBool();
					else if (aChild->m_name == "map_entity_spawn")
						m_mapEntitySpawnId = aChild->GetId(DataType::ID_MAP_ENTITY_SPAWN);
					else if (aChild->m_name == "tag_contexts")
						aChild->GetIdArray(DataType::ID_TAG_CONTEXT, m_tagContextIds);
					else if (aChild->m_name == "influence_range")
						m_influenceRange = UIntRange(aChild);
					else if(aChild->m_name == "random_object")	
						m_randomObject = RandomObject(aChild);
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*								aWriter) const
			{
				aWriter->WriteUInts(m_tagContextIds);
				aWriter->WriteUInt(m_minDistanceToNearBy);
				aWriter->WriteUInt(m_probability);
				aWriter->WriteBool(m_elite);
				aWriter->WriteUInt(m_mapEntitySpawnId);
				m_influenceRange.ToStream(aWriter);
				aWriter->WriteOptionalObject(m_randomObject);
			}

			bool
			FromStream(
				IReader*								aReader)
			{
				if (!aReader->ReadUInts(m_tagContextIds))
					return false;
				if (!aReader->ReadUInt(m_minDistanceToNearBy))
					return false;
				if (!aReader->ReadUInt(m_probability))
					return false;
				if(!aReader->ReadBool(m_elite))
					return false;
				if (!aReader->ReadUInt(m_mapEntitySpawnId))
					return false;
				if (!m_influenceRange.FromStream(aReader))
					return false;
				if(!aReader->ReadOptionalObject(m_randomObject))
					return false;
				return true;
			}

			// Public data
			std::vector<uint32_t>					m_tagContextIds;
			uint32_t								m_minDistanceToNearBy = 0;
			bool									m_elite = false;
			uint32_t								m_mapEntitySpawnId = 0;
			UIntRange								m_influenceRange;
			uint32_t								m_probability = 0;
			std::optional<RandomObject>				m_randomObject;
		};

		struct SpecialEntity
		{
			enum Placement : uint8_t
			{
				INVALID_PLACEMENT,

				PLACEMENT_FAR_AWAY_FROM_PLAYER_SPAWNS,
				PLACEMENT_IN_PLAYER_SPAWN_DISTANCE_RANGE,
				PLACEMENT_IN_PLAYER_SPAWN_DISTANCE_RANGE_PERCENTAGE,
			};

			SpecialEntity()
			{

			}

			SpecialEntity(
				const SourceNode*						aSource)
			{
				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_annotation)
					{
						if(aChild->m_annotation->m_children.size() == 2 && aChild->m_annotation->IsArrayType(SourceNode::TYPE_NUMBER))
							m_range = UIntRange(aChild->m_annotation.get());
						else
							TP_VERIFY(false, aChild->m_annotation->m_debugInfo, "Invalid annotation.");
					}

					if(aChild->m_name == "placement" && aChild->IsIdentifier("far_away_from_player_spawns"))
						m_placement = PLACEMENT_FAR_AWAY_FROM_PLAYER_SPAWNS;
					else if (aChild->m_name == "placement" && aChild->IsIdentifier("in_player_spawn_distance_range"))
						m_placement = PLACEMENT_IN_PLAYER_SPAWN_DISTANCE_RANGE;
					else if (aChild->m_name == "placement" && aChild->IsIdentifier("in_player_spawn_distance_range_percentage"))
						m_placement = PLACEMENT_IN_PLAYER_SPAWN_DISTANCE_RANGE_PERCENTAGE;
					else if(aChild->m_name == "entity_spawn")
						aChild->GetIdArray(DataType::ID_MAP_ENTITY_SPAWN, m_mapEntitySpawns);
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*								aWriter) const
			{
				aWriter->WritePOD(m_placement);
				aWriter->WriteUInts(m_mapEntitySpawns);
				aWriter->WriteOptionalObject(m_range);
			}

			bool
			FromStream(
				IReader*								aReader)
			{
				if(!aReader->ReadPOD(m_placement))
					return false;
				if(!aReader->ReadUInts(m_mapEntitySpawns))
					return false;
				if(!aReader->ReadOptionalObject(m_range))
					return false;
				return true;
			}

			// Public data
			Placement								m_placement = INVALID_PLACEMENT;
			std::vector<uint32_t>					m_mapEntitySpawns;
			std::optional<UIntRange>				m_range;
		};

		struct Builder
		{
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
			void			UpdateZoneMap(
								const Vec2&					aPosition,
								uint32_t					aZoneId);
			void			InitBosses();
			void			InitPlayerSpawns();
			void			InitSpecialEntities();
			void			InitRoutes();
			void			InitZones();

			// Public data
			const Params*									m_params = NULL;
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
				uint32_t									m_id = 0;
				std::set<Vec2>								m_positions;
				bool										m_complete = false;
			};

			std::vector<std::unique_ptr<WalkableArea>>		m_walkableAreas;
			std::set<Vec2>									m_walkable;

			static const int32_t NOISE_MAP_SIZE = 32;
			uint32_t										m_noiseMap[NOISE_MAP_SIZE * NOISE_MAP_SIZE];			

			std::vector<MapData::EntitySpawn>				m_entitySpawns;
			std::set<Vec2>									m_entitySpawnPositions;

			struct MinorBoss
			{
				const Data::Entity*							m_entity = NULL;
				Vec2										m_position;
			};

			struct Boss
			{
				uint32_t									m_tagContextId = 0;
				uint32_t									m_influence = 32;
				uint32_t									m_mapEntitySpawnId = 0;
				const MinorBosses*							m_minorBosses = NULL;
				std::vector<const Pack*>					m_packs;
				const Data::Entity*							m_entity = NULL;
				std::unique_ptr<DistanceField>				m_distanceField;
				Vec2										m_position;
				std::vector<MinorBoss>						m_subBosses;
				uint32_t									m_factionId = 0;
				uint32_t									m_influenceTileTransformThreshold = 0;
			};

			struct LevelMapPoint
			{
				const Boss*									m_boss = NULL;
				uint32_t									m_influence = 0;
				uint32_t									m_level = 0;
			};

			struct PlayerSpawn
			{
				Vec2										m_position;
			};

			struct Doodad
			{
				Vec2										m_position;
				uint32_t									m_doodadId = 0;
			};

			struct Route
			{
				enum Type
				{
					INVALID_TYPE, 

					TYPE_RANDOM
				};

				Type										m_type = INVALID_TYPE;
				uint32_t									m_routeId = 0;				
				Vec2										m_from;
				Vec2										m_to;
				std::vector<Vec2>							m_positions;
			};

			struct PlayerSpawnZoneBoundaryEntity
			{
				UIntRange									m_range;
				uint32_t									m_entityId = 0;
			};

			std::vector<std::unique_ptr<Boss>>				m_bosses;
			std::unique_ptr<DistanceField>					m_bossDistanceCombined;
			std::vector<std::unique_ptr<PlayerSpawn>>		m_playerSpawns;
			std::unique_ptr<DistanceField>					m_playerSpawnDistanceCombined;
			std::vector<LevelMapPoint>						m_levelMap;
			std::vector<Doodad>								m_doodads;
			std::vector<SpecialEntity>						m_specialEntities;
			std::vector<std::unique_ptr<Route>>				m_routes;
			std::vector<uint32_t>							m_zoneMap;
			std::vector<PlayerSpawnZoneBoundaryEntity>		m_playerSpawnZoneBoundaryEntities;
		
			UIntRange										m_levelRange;
		};

		struct IExecute
		{
			enum Type : uint8_t
			{
				INVALID_TYPE,

				TYPE_ALL,
				TYPE_ONE_OF,
				TYPE_PALETTED_MAP,
				TYPE_DOUBLE,
				TYPE_MEDIAN_FILTER,
				TYPE_GROW,
				TYPE_DESPECKLE,
				TYPE_ADD_BORDERS,
				TYPE_DEBUG_TERRAIN_MAP,
				TYPE_OVERLAY,
				TYPE_TERRAIN_MODIFIER_MAP,
				TYPE_ADD_ENTITY_SPAWNS,
				TYPE_ADD_BOSS,
				TYPE_LEVEL_RANGE,
				TYPE_PLAYER_SPAWNS,
				TYPE_ADD_SPECIAL_ENTITY_SPAWN,
				TYPE_ADD_RANDOM_ROUTE,
				TYPE_ADD_PLAYER_SPAWN_ZONE_BOUNDARY_ENTITY,

				NUM_TYPES
			};

			static Type
			SourceToType(
				const SourceNode*	aSource)
			{
				std::string_view t(aSource->m_name.c_str());
				if (t == "all")
					return TYPE_ALL;
				else if (t == "one_of")
					return TYPE_ONE_OF;
				else if (t == "paletted_map")
					return TYPE_PALETTED_MAP;
				else if (t == "double")
					return TYPE_DOUBLE;
				else if (t == "median_filter")
					return TYPE_MEDIAN_FILTER;
				else if (t == "grow")
					return TYPE_GROW;
				else if (t == "despeckle")
					return TYPE_DESPECKLE;
				else if (t == "add_borders")
					return TYPE_ADD_BORDERS;
				else if (t == "debug_terrain_map")
					return TYPE_DEBUG_TERRAIN_MAP;
				else if (t == "overlay")
					return TYPE_OVERLAY;
				else if (t == "terrain_modifier_map")
					return TYPE_TERRAIN_MODIFIER_MAP;
				else if (t == "add_entity_spawns")
					return TYPE_ADD_ENTITY_SPAWNS;
				else if (t == "add_boss")
					return TYPE_ADD_BOSS;
				else if (t == "level_range")
					return TYPE_LEVEL_RANGE;
				else if (t == "player_spawns")
					return TYPE_PLAYER_SPAWNS;
				else if (t == "add_special_entity_spawn")
					return TYPE_ADD_SPECIAL_ENTITY_SPAWN;
				else if(t == "add_random_route")
					return TYPE_ADD_RANDOM_ROUTE;
				else if (t == "add_player_spawn_zone_boundary_entity")
					return TYPE_ADD_PLAYER_SPAWN_ZONE_BOUNDARY_ENTITY;
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid execute type.", aSource->m_name.c_str());
				return INVALID_TYPE;
			}

			struct Header
			{
				void
				ToStream(
					IWriter*							aWriter) const
				{
					aWriter->WritePOD(m_type);
					aWriter->WriteUInt(m_value);
				}

				bool
				FromStream(				
					IReader*							aReader)
				{
					if(!aReader->ReadPOD(m_type))
						return false;
					if(!aReader->ReadUInt(m_value))
						return false;
					return true;
				}

				// Public data
				Type								m_type = INVALID_TYPE;
				uint32_t							m_value = 1;
			};

			IExecute(
				Type		aType)
			{
				m_header.m_type = aType;
			}

			virtual 
			~IExecute() 
			{

			}

			// Virtual interface
			virtual void		FromSource(
									const IExecuteFactory*	aFactory,
									const SourceNode*		aSource) = 0;
			virtual void		ToStream(
									IWriter*				aWriter) const = 0;
			virtual bool		FromStream(
									const IExecuteFactory*	aFactory,
									IReader*				aReader) = 0;
			virtual void		Run(
									Builder*				aBuilder) const = 0;

			// Public data
			Header									m_header;
		};

		struct IExecuteFactory
		{
			IExecuteFactory();
			~IExecuteFactory();

			template <typename _T>
			void
			Register()
			{
				m_callbacks[_T::TYPE] = []() { return new _T(); };
			}

			IExecute*
			Create(
				IExecute::Type							aType) const
			{		
				if((uint32_t)aType >= (uint32_t)IExecute::NUM_TYPES)
					return NULL;
				assert(m_callbacks[aType]);
				return m_callbacks[aType]();
			}

			// Public data
			typedef std::function<IExecute*()> Callback;
			Callback								m_callbacks[IExecute::NUM_TYPES];
		};

		struct CompoundExecute
		{
			void				
			ToStream(
				IWriter*								aWriter) const
			{
				aWriter->WriteUInt(m_children.size());
				for(const std::unique_ptr<IExecute>& t : m_children)
				{
					t->m_header.ToStream(aWriter);
					t->ToStream(aWriter);
				}
			}

			bool					
			FromStream(
				const IExecuteFactory*					aFactory,
				IReader*								aReader) 
			{
				size_t count;
				if(!aReader->ReadUInt(count))
					return false;

				for(size_t i = 0; i < count; i++)
				{
					IExecute::Header header;
					if(!header.FromStream(aReader))
						return false;

					std::unique_ptr<IExecute> t(aFactory->Create(header.m_type));
					if(!t)
						return false;

					if(!t->FromStream(aFactory, aReader))
						return false;

					m_children.push_back(std::move(t));
				}
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<IExecute>>	m_children;
		};

		struct ExecuteAll : public IExecute
		{
			static const Type TYPE = TYPE_ALL;
								
			ExecuteAll() : IExecute(TYPE) { }
			virtual	~ExecuteAll() { }

			// IExecute implementation
			void				
			FromSource(
				const IExecuteFactory*					aFactory,
				const SourceNode*						aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					std::unique_ptr<IExecute> t(aFactory->Create(IExecute::SourceToType(aChild)));
					t->FromSource(aFactory, aChild);

					if(aChild->m_annotation)
						t->m_header.m_value = aChild->m_annotation->GetUInt32();

					m_compound.m_children.push_back(std::move(t));
				});
			}

			void				
			ToStream(
				IWriter*								aWriter) const override
			{
				m_compound.ToStream(aWriter);
			}

			bool					
			FromStream(
				const IExecuteFactory*					aFactory,
				IReader*								aReader) override
			{
				if(!m_compound.FromStream(aFactory, aReader))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			CompoundExecute						m_compound;
		};

		struct ExecuteOneOf : public IExecute
		{
			static const Type TYPE = TYPE_ONE_OF;

			ExecuteOneOf() : IExecute(TYPE) { }
			virtual	~ExecuteOneOf() { }

			// IExecute implementation
			void				
			FromSource(
				const IExecuteFactory*					aFactory,
				const SourceNode*						aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					std::unique_ptr<IExecute> t(aFactory->Create(IExecute::SourceToType(aChild)));
					t->FromSource(aFactory, aChild);

					if (aChild->m_annotation)
						t->m_header.m_value = aChild->m_annotation->GetUInt32();

					m_compound.m_children.push_back(std::move(t));
				});
			}

			void				
			ToStream(
				IWriter*								aWriter) const override
			{
				m_compound.ToStream(aWriter);
			}

			bool					
			FromStream(
				const IExecuteFactory*					aFactory,
				IReader*								aReader) override
			{
				if(!m_compound.FromStream(aFactory, aReader))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			CompoundExecute						m_compound;
		};

		struct ExecutePalettedMap : public IExecute
		{
			static const Type TYPE = TYPE_PALETTED_MAP;

			ExecutePalettedMap() : IExecute(TYPE) { }
			virtual	~ExecutePalettedMap() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
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
				IWriter*							aWriter) const override
			{
				TP_CHECK(m_tiles.size() > 0 && m_width * m_height == (uint32_t)m_tiles.size(), "Invalid paletted map.");
				aWriter->WriteUInt(m_width);
				aWriter->WriteUInt(m_height);
				aWriter->Write(&m_tiles[0], m_tiles.size());
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
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

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			uint32_t								m_width = 0;
			uint32_t								m_height = 0;
			std::vector<char>						m_tiles;
		};

		struct ExecuteDouble : public IExecute
		{
			static const Type TYPE = TYPE_DOUBLE;

			ExecuteDouble() : IExecute(TYPE) { }
			virtual	~ExecuteDouble() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "passes")
						m_passes = aChild->GetUInt32();
					else if(aChild->m_name == "no_mutations")
						m_noMutations = aChild->GetBool();
					else 
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*							aWriter) const override
			{
				aWriter->WriteUInt(m_passes);
				aWriter->WriteBool(m_noMutations);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if(!aReader->ReadUInt(m_passes))
					return false;
				if(!aReader->ReadBool(m_noMutations))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			uint32_t		m_passes = 1;
			bool			m_noMutations = false;
		};

		struct ExecuteMedianFilter : public IExecute
		{
			static const Type TYPE = TYPE_MEDIAN_FILTER;

			ExecuteMedianFilter() : IExecute(TYPE) { }
			virtual	~ExecuteMedianFilter() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{
				m_radius = aSource->GetUInt32();
			}

			void
			ToStream(
				IWriter*							aWriter) const override
			{
				aWriter->WriteUInt(m_radius);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if(!aReader->ReadUInt(m_radius))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			uint32_t		m_radius = 1;
		};

		struct ExecuteGrow : public IExecute
		{
			static const Type TYPE = TYPE_GROW;

			ExecuteGrow() : IExecute(TYPE) { }
			virtual	~ExecuteGrow() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{
				m_terrainId = aSource->GetId(DataType::ID_TERRAIN);
			}

			void
			ToStream(
				IWriter*							aWriter) const override
			{
				aWriter->WriteUInt(m_terrainId);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if(!aReader->ReadUInt(m_terrainId))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			uint32_t			m_terrainId = 0;
		};

		struct ExecuteDespeckle : public IExecute
		{
			static const Type TYPE = TYPE_DESPECKLE;

			ExecuteDespeckle() : IExecute(TYPE) { }
			virtual	~ExecuteDespeckle() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "from_terrain")
						m_fromTerrainId = aChild->GetId(DataType::ID_TERRAIN);
					else if(aChild->m_name == "to_terrain")
						m_toTerrainId = aChild->GetId(DataType::ID_TERRAIN);
					else 
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*							aWriter) const override
			{
				aWriter->WriteUInt(m_fromTerrainId);
				aWriter->WriteUInt(m_toTerrainId);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if(!aReader->ReadUInt(m_fromTerrainId))
					return false;
				if (!aReader->ReadUInt(m_toTerrainId))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			uint32_t			m_fromTerrainId = 0;
			uint32_t			m_toTerrainId = 0;
		};

		struct ExecuteAddBorders : public IExecute
		{
			static const Type TYPE = TYPE_ADD_BORDERS;

			ExecuteAddBorders() : IExecute(TYPE) { }
			virtual	~ExecuteAddBorders() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{
				m_terrainId = aSource->GetId(DataType::ID_TERRAIN);
			}

			void
			ToStream(
				IWriter*							aWriter) const override
			{
				aWriter->WriteUInt(m_terrainId);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if (!aReader->ReadUInt(m_terrainId))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			uint32_t			m_terrainId = 0;
		};

		struct ExecuteDebugTerrainMap : public IExecute
		{
			static const Type TYPE = TYPE_DEBUG_TERRAIN_MAP;

			ExecuteDebugTerrainMap() : IExecute(TYPE) { }
			virtual	~ExecuteDebugTerrainMap() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{
				m_path = aSource->GetString();
			}

			void
			ToStream(
				IWriter*							aWriter) const override
			{
				aWriter->WriteString(m_path);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if(!aReader->ReadString(m_path))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			std::string			m_path;
		};

		struct ExecuteOverlay : public IExecute
		{
			static const Type TYPE = TYPE_OVERLAY;

			ExecuteOverlay() : IExecute(TYPE) { }
			virtual	~ExecuteOverlay() { }

			struct Condition
			{
				enum Type
				{
					INVALID_TYPE,

					TYPE_ABOVE,
					TYPE_BELOW,
					TYPE_RANGE
				};

				Condition()
				{

				}

				Condition(
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

			struct Terrain
			{
				Terrain()
				{

				}

				Terrain(
					const SourceNode*					aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing terrain annotation.");
					m_terrainId = aSource->m_annotation->GetId(DataType::ID_TERRAIN);

					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if (aChild->m_name == "above")
							m_condition = Condition(Condition::TYPE_ABOVE, aChild);
						else if (aChild->m_name == "below")
							m_condition = Condition(Condition::TYPE_BELOW, aChild);
						else if (aChild->m_name == "range")
							m_condition = Condition(Condition::TYPE_RANGE, aChild);
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
					if (!m_condition.FromStream(aReader))
						return false;
					if (!aReader->ReadUInts(m_exclude))
						return false;
					return true;
				}

				bool
				IsExcluded(
					uint32_t							aTerrainId) const
				{
					for (uint32_t t : m_exclude)
					{
						if (t == aTerrainId)
							return true;
					}
					return false;
				}

				// Public data
				uint32_t				m_terrainId = 0;
				Condition				m_condition;
				std::vector<uint32_t>	m_exclude;
			};

			struct Doodad
			{
				Doodad()
				{

				}

				Doodad(
					const SourceNode*					aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing doodad annotation.");
					m_doodadId = aSource->m_annotation->GetId(DataType::ID_DOODAD);

					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if (aChild->m_name == "range")
							m_range = UIntRange(aChild);
						else if (aChild->m_name == "probability")
							m_probability = UIntRange(aChild);
						else if (aChild->m_name == "terrain")
							aChild->GetIdArray(DataType::ID_TERRAIN, m_terrainIds);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*							aWriter) const 
				{
					aWriter->WriteUInt(m_doodadId);
					aWriter->WriteUInts(m_terrainIds);
					m_range.ToStream(aWriter);
					m_probability.ToStream(aWriter);					
				}

				bool
				FromStream(
					IReader*							aReader) 
				{
					if (!aReader->ReadUInt(m_doodadId))
						return false;
					if (!aReader->ReadUInts(m_terrainIds))
						return false;
					if (!m_range.FromStream(aReader))
						return false;
					if (!m_probability.FromStream(aReader))
						return false;
					return true;
				}

				bool
				HasTerrainId(
					uint32_t							aTerrainId) const
				{
					for(uint32_t t : m_terrainIds)
					{
						if(t == aTerrainId)
							return true;
					}
					return false;
				}

				// Public data
				uint32_t				m_doodadId = 0;
				UIntRange				m_range;
				UIntRange				m_probability;
				std::vector<uint32_t>	m_terrainIds;
			};

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if (aChild->m_name == "noise")
						m_noiseId = aChild->GetId(DataType::ID_NOISE);
					else if (aChild->m_name == "terrain")
						m_terrains.push_back(std::make_unique<Terrain>(aChild));
					else if (aChild->m_name == "doodad")
						m_doodads.push_back(std::make_unique<Doodad>(aChild));
					else if(aChild->m_name == "border")
						m_border = aChild->GetUInt32();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});

			}

			void
			ToStream(
				IWriter*							aWriter) const override
			{
				aWriter->WriteUInt(m_noiseId);
				aWriter->WriteObjectPointers(m_terrains);
				aWriter->WriteObjectPointers(m_doodads);
				aWriter->WriteUInt(m_border);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if (!aReader->ReadUInt(m_noiseId))
					return false;
				if (!aReader->ReadObjectPointers(m_terrains))
					return false;
				if (!aReader->ReadObjectPointers(m_doodads))
					return false;
				if (!aReader->ReadUInt(m_border))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			uint32_t										m_noiseId = 0;
			std::vector<std::unique_ptr<Terrain>>			m_terrains;
			std::vector<std::unique_ptr<Doodad>>			m_doodads;
			uint32_t										m_border = 0;
		};

		struct ExecuteTerrainModifierMap : public IExecute
		{
			static const Type TYPE = TYPE_TERRAIN_MODIFIER_MAP;

			ExecuteTerrainModifierMap() : IExecute(TYPE) { }
			virtual	~ExecuteTerrainModifierMap() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "noise")
					{
						m_noiseId = aChild->GetId(DataType::ID_NOISE);
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
				IWriter*							aWriter) const override
			{
				aWriter->WritePOD(m_id);
				aWriter->WriteUInt(m_noiseId);
				aWriter->WriteString(m_debug);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if (!aReader->ReadPOD(m_id))
					return false;
				if (!aReader->ReadUInt(m_noiseId))
					return false;
				if (!aReader->ReadString(m_debug))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			TerrainModifier::Id								m_id = TerrainModifier::INVALID_ID;
			uint32_t										m_noiseId = 0;
			std::string										m_debug;
		};

		struct ExecuteAddEntitySpawns : public IExecute
		{
			static const Type TYPE = TYPE_ADD_ENTITY_SPAWNS;

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
						m_terrainId = aSource->m_annotation->GetId(DataType::ID_TERRAIN);
						m_probabilityBonus = aSource->GetUInt32();
					}
					else if(aSource->m_name == "neighbor_terrain_required")
					{
						m_terrainId = aSource->GetId(DataType::ID_TERRAIN);
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

			ExecuteAddEntitySpawns() : IExecute(TYPE) { }
			virtual	~ExecuteAddEntitySpawns() { }

			bool
			CheckTerrain(
				uint32_t							aTerrainId) const
			{
				for(uint32_t t : m_terrains)
				{
					if(t == aTerrainId)
						return true;
				}
				return false;
			}

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
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
				IWriter*							aWriter) const override
			{
				aWriter->WriteUInts(m_mapEntitySpawns);
				aWriter->WriteUInt(m_probability);
				aWriter->WriteUInts(m_terrains);
				aWriter->WriteObjects(m_neighborTerrains);
				aWriter->WriteUInt(m_minDistanceToNearby);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
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

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			std::vector<uint32_t>							m_mapEntitySpawns;
			uint32_t										m_probability = 0;
			std::vector<uint32_t>							m_terrains;
			std::vector<NeighborTerrain>					m_neighborTerrains;
			uint32_t										m_minDistanceToNearby = 0;
			bool											m_needWalkable = false;
		};

		struct ExecuteAddBoss : public IExecute
		{
			static const Type TYPE = TYPE_ADD_BOSS;			

			ExecuteAddBoss() : IExecute(TYPE) { }
			virtual	~ExecuteAddBoss() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if (aChild->m_name == "tag_context")
						m_tagContextId = aChild->GetId(DataType::ID_TAG_CONTEXT);
					else if (aChild->m_name == "map_entity_spawn")
						m_mapEntitySpawnId = aChild->GetId(DataType::ID_MAP_ENTITY_SPAWN);
					else if (aChild->m_name == "influence")
						m_influence = aChild->GetUInt32();
					else if (aChild->m_name == "influence_tile_transform_threshold")
						m_influenceTileTransformThreshold = UIntRange(aChild);
					else if(aChild->m_name == "minor_bosses")
						m_minorBosses = std::make_unique<MinorBosses>(aChild);
					else if(aChild->m_name == "pack")
						m_packs.push_back(std::make_unique<Pack>(aChild));
					else 
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*							aWriter) const override
			{
				aWriter->WriteOptionalObjectPointer(m_minorBosses);
				aWriter->WriteUInt(m_tagContextId);
				aWriter->WriteUInt(m_mapEntitySpawnId);
				aWriter->WriteUInt(m_influence);
				m_influenceTileTransformThreshold.ToStream(aWriter);
				aWriter->WriteObjectPointers(m_packs);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if(!aReader->ReadOptionalObjectPointer(m_minorBosses))
					return false;
				if (!aReader->ReadUInt(m_tagContextId))
					return false;
				if (!aReader->ReadUInt(m_mapEntitySpawnId))
					return false;
				if (!aReader->ReadUInt(m_influence))
					return false;
				if(!m_influenceTileTransformThreshold.FromStream(aReader))
					return false;
				if(!aReader->ReadObjectPointers(m_packs))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			std::unique_ptr<MinorBosses>				m_minorBosses;
			uint32_t									m_tagContextId = 0;
			uint32_t									m_mapEntitySpawnId = 0;
			uint32_t									m_influence = 0;
			UIntRange									m_influenceTileTransformThreshold;
			std::vector<std::unique_ptr<Pack>>			m_packs;
		};

		struct ExecuteLevelRange : public IExecute
		{
			static const Type TYPE = TYPE_LEVEL_RANGE;

			ExecuteLevelRange() : IExecute(TYPE) { }
			virtual	~ExecuteLevelRange() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{
				m_levelRange = UIntRange(aSource);
			}

			void
			ToStream(
				IWriter*							aWriter) const override
			{
				m_levelRange.ToStream(aWriter);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if(!m_levelRange.FromStream(aReader))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			UIntRange									m_levelRange;
		};
		
		struct ExecutePlayerSpawns : public IExecute
		{
			static const Type TYPE = TYPE_PLAYER_SPAWNS;

			ExecutePlayerSpawns() : IExecute(TYPE) { }
			virtual	~ExecutePlayerSpawns() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{
				m_count = aSource->GetUInt32();
			}

			void
			ToStream(
				IWriter*							aWriter) const override
			{
				aWriter->WriteUInt(m_count);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if(!aReader->ReadUInt(m_count))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			uint32_t									m_count = 1;
		};

		struct ExecuteAddSpecialEntitySpawn : public IExecute
		{
			static const Type TYPE = TYPE_ADD_SPECIAL_ENTITY_SPAWN;

			ExecuteAddSpecialEntitySpawn() : IExecute(TYPE) { }
			virtual	~ExecuteAddSpecialEntitySpawn() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{
				m_specialEntity = SpecialEntity(aSource);
			}

			void
			ToStream(
				IWriter*							aWriter) const override
			{
				m_specialEntity.ToStream(aWriter);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if(!m_specialEntity.FromStream(aReader))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			SpecialEntity				m_specialEntity;
		};

		struct ExecuteAddRandomRoute : public IExecute
		{
			static const Type TYPE = TYPE_ADD_RANDOM_ROUTE;

			ExecuteAddRandomRoute() : IExecute(TYPE) { }
			virtual	~ExecuteAddRandomRoute() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{				
				m_routeId = aSource->GetId(DataType::ID_ROUTE);
			}

			void
			ToStream(
				IWriter*							aWriter) const override
			{
				aWriter->WriteUInt(m_routeId);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if(!aReader->ReadUInt(m_routeId))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			uint32_t					m_routeId = 0;
		};

		struct ExecuteAddPlayerSpawnZoneBoundaryEntity : public IExecute
		{
			static const Type TYPE = TYPE_ADD_PLAYER_SPAWN_ZONE_BOUNDARY_ENTITY;

			ExecuteAddPlayerSpawnZoneBoundaryEntity() : IExecute(TYPE) { }
			virtual	~ExecuteAddPlayerSpawnZoneBoundaryEntity() { }

			// IExecute implementation
			void
			FromSource(
				const IExecuteFactory*				/*aFactory*/,
				const SourceNode*					aSource) override
			{				
				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "range")					
						m_range = UIntRange(aChild);
					else if(aChild->m_name == "entity")
						m_entityId = aChild->GetId(DataType::ID_ENTITY);
					else 
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*							aWriter) const override
			{
				m_range.ToStream(aWriter);
				aWriter->WriteUInt(m_entityId);
			}

			bool
			FromStream(
				const IExecuteFactory*				/*aFactory*/,
				IReader*							aReader) override
			{
				if (!m_range.FromStream(aReader))
					return false;
				if (!aReader->ReadUInt(m_entityId))
					return false;
				return true;
			}

			void				Run(
									Builder*			aBuilder) const override;

			// Public data
			UIntRange					m_range;
			uint32_t					m_entityId = 0;
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

		IExecuteFactory										m_executeFactory;
		TerrainPalette										m_terrainPalette;
		
		Params												m_params;
		CompoundExecute										m_executes;

		void			_AddTerrainPaletteEntry(
							const SourceNode*			aSource);
	};

}