#pragma once

#include "EntityState.h"
#include "IReader.h"
#include "IWriter.h"
#include "MapCovers.h"
#include "MapGeneratorBase.h"
#include "MapGeneratorFactory.h"
#include "MapInfo.h"
#include "MapType.h"
#include "Parser.h"

namespace tpublic
{

	class AutoDoodads;
	class Manifest;
	class MapPathData;
	class MapRouteData;
	class WorldInfoMap;

	class MapData
	{
	public:
		struct PVPSwapReputationsWithAura
		{
			PVPSwapReputationsWithAura()
			{

			}

			PVPSwapReputationsWithAura(
				const SourceNode*			aSource)
			{
				m_firstFactionId = aSource->GetAnnotation()->GetArrayIndex(0)->GetId(DataType::ID_FACTION);
				m_secondFactionId = aSource->GetAnnotation()->GetArrayIndex(1)->GetId(DataType::ID_FACTION);
				m_auraId = aSource->GetId(DataType::ID_AURA);
			}

			void
			ToStream(
				IWriter*					aWriter) const
			{
				aWriter->WriteUInt(m_firstFactionId);
				aWriter->WriteUInt(m_secondFactionId);
				aWriter->WriteUInt(m_auraId);
			}

			bool
			FromStream(
				IReader*					aReader)
			{
				if (!aReader->ReadUInt(m_firstFactionId))
					return false;
				if (!aReader->ReadUInt(m_secondFactionId))
					return false;
				if (!aReader->ReadUInt(m_auraId))
					return false;
				return true;
			}

			// Public data
			uint32_t		m_firstFactionId = 0;
			uint32_t		m_secondFactionId = 0;
			uint32_t		m_auraId = 0;
		};

		struct PVPControlPointRealmBalanceUpdate
		{
			PVPControlPointRealmBalanceUpdate()
			{

			}

			PVPControlPointRealmBalanceUpdate(
				const SourceNode*			aSource)
			{
				m_realmBalanceId = aSource->GetAnnotation()->GetId(DataType::ID_REALM_BALANCE);
				
				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "entities")
						aChild->GetIdArray(DataType::ID_ENTITY, m_entityIds);
					else if(aChild->m_name == "interval")
						m_interval = aChild->GetUInt32();
					else if(aChild->m_tag == "control_point_state")
						m_controlPointStates[aChild->GetAnnotation()->GetId(DataType::ID_CONTROL_POINT_STATE)] = aChild->GetInt32();
					else 
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*					aWriter) const
			{
				aWriter->WriteUInt(m_realmBalanceId);
				aWriter->WriteUInts(m_entityIds);
				aWriter->WriteUIntToIntTable<uint32_t, int32_t>(m_controlPointStates);
				aWriter->WriteUInt(m_interval);
			}

			bool
			FromStream(
				IReader*					aReader)
			{
				if (!aReader->ReadUInt(m_realmBalanceId))
					return false;
				if (!aReader->ReadUInts(m_entityIds))
					return false;
				if (!aReader->ReadUIntToIntTable<uint32_t, int32_t>(m_controlPointStates))
					return false;
				if (!aReader->ReadUInt(m_interval))
					return false;
				return true;
			}

			int32_t
			Get(
				uint32_t					aControlPointStateId) const
			{
				std::unordered_map<uint32_t, int32_t>::const_iterator i = m_controlPointStates.find(aControlPointStateId);
				if(i == m_controlPointStates.cend())
					return 0;
				return i->second;
			}

			// Public data
			uint32_t									m_realmBalanceId = 0;
			std::vector<uint32_t>						m_entityIds;
			std::unordered_map<uint32_t, int32_t>		m_controlPointStates;
			uint32_t									m_interval = 0;
		};

		struct PVPFaction
		{
			PVPFaction()
			{

			}

			PVPFaction(
				const SourceNode*			aSource)
			{
				m_factionId = aSource->m_sourceContext->m_persistentIdTable->GetId(aSource->m_debugInfo, DataType::ID_FACTION, aSource->m_name.c_str());

				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "reputation")
						m_reputationFactionId = aChild->GetId(DataType::ID_FACTION);
					else if(aChild->m_name == "player_spawn")
						m_playerSpawnId = aChild->GetId(DataType::ID_MAP_PLAYER_SPAWN);
					else if(aChild->m_name == "control_point_entities")
						aChild->GetIdArray(DataType::ID_ENTITY, m_controlPointEntityIds);
					else if (aChild->m_name == "captured_control_point_state")
						m_capturedControlPointStateId = aChild->GetId(DataType::ID_CONTROL_POINT_STATE);
					else					
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*					aWriter) const
			{
				aWriter->WriteUInt(m_factionId);
				aWriter->WriteUInt(m_reputationFactionId);
				aWriter->WriteUInt(m_playerSpawnId);
				aWriter->WriteUInts(m_controlPointEntityIds);
				aWriter->WriteUInt(m_capturedControlPointStateId);
			}

			bool
			FromStream(
				IReader*					aReader)
			{
				if (!aReader->ReadUInt(m_factionId))
					return false;
				if (!aReader->ReadUInt(m_reputationFactionId))
					return false;
				if (!aReader->ReadUInt(m_playerSpawnId))
					return false;
				if (!aReader->ReadUInts(m_controlPointEntityIds))
					return false;
				if (!aReader->ReadUInt(m_capturedControlPointStateId))
					return false;
				return true;
			}

			// Public data
			uint32_t									m_factionId = 0;
			uint32_t									m_reputationFactionId = 0;
			uint32_t									m_playerSpawnId = 0;
			std::vector<uint32_t>						m_controlPointEntityIds;
			uint32_t									m_capturedControlPointStateId = 0;
		};

		enum PVPType : uint8_t
		{
			INVALID_PVP_TYPE,

			PVP_TYPE_REPUTATION
		};

		struct PVP
		{
			PVP()
			{

			}

			PVP(
				const SourceNode*			aSource)
			{
				TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing PVP type annotation.");
				std::string_view t(aSource->m_annotation->GetIdentifier());
				if(t == "reputation")
					m_type = PVP_TYPE_REPUTATION;
				else
					TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid PVP type.", aSource->m_annotation->GetIdentifier());

				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "swap_reputations_with_aura")
						m_swapReputationsWithAura = PVPSwapReputationsWithAura(aChild);
					else if (aChild->m_tag == "faction")
						m_factions.push_back(std::make_unique<PVPFaction>(aChild));
					else if (aChild->m_tag == "control_point_realm_balance_update")
						m_controlPointRealmBalanceUpdates.push_back(std::make_unique<PVPControlPointRealmBalanceUpdate>(aChild));
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*					aWriter) const
			{
				aWriter->WritePOD(m_type);
				aWriter->WriteObjectPointers(m_factions);
				aWriter->WriteObjectPointers(m_controlPointRealmBalanceUpdates);
				aWriter->WriteOptionalObject(m_swapReputationsWithAura);
			}

			bool
			FromStream(
				IReader*					aReader)
			{
				if(!aReader->ReadPOD(m_type))
					return false;
				if (!aReader->ReadObjectPointers(m_factions))
					return false;
				if (!aReader->ReadObjectPointers(m_controlPointRealmBalanceUpdates))
					return false;
				if (!aReader->ReadOptionalObject(m_swapReputationsWithAura))
					return false;
				return true;
			}

			const PVPFaction*
			GetFaction(
				uint32_t					aFactionId) const
			{
				for(const std::unique_ptr<PVPFaction>& faction : m_factions)
				{
					if(faction->m_factionId == aFactionId)
						return faction.get();
				}
				return NULL;
			}

			// Public data
			PVPType															m_type = INVALID_PVP_TYPE;
			std::vector<std::unique_ptr<PVPFaction>>						m_factions;
			std::vector<std::unique_ptr<PVPControlPointRealmBalanceUpdate>>	m_controlPointRealmBalanceUpdates;
			std::optional<PVPSwapReputationsWithAura>						m_swapReputationsWithAura;
		};

		enum SeedType : uint8_t
		{
			INVALID_SEED_TYPE,

			SEED_TYPE_TIME_SEED,	// Based on TimeSeed object
			SEED_TYPE_RANDOM,		// New random seed every time
			SEED_TYPE_CONSTANT,		// Always same seed (m_value)
			SEED_TYPE_HOURLY,		// Based on server date (year-month-day-hour)
			SEED_TYPE_DAILY,		// Based on server date (year-month-day)
			SEED_TYPE_WEEKLY,		// Based on server date (year-week)
			SEED_TYPE_MONTHLY,		// Based on server date (year-month)
			SEED_TYPE_YEARLY		// Based on server date (year)
		};

		struct Seed
		{		
			void
			FromSource(
				const SourceNode*			aSource)
			{
				if(aSource->m_type == SourceNode::TYPE_NUMBER)
				{
					m_constant = aSource->GetUInt32();
					m_type = SEED_TYPE_CONSTANT;
				}
				else
				{
					std::string_view seedTypeString(aSource->GetIdentifier());
					if (seedTypeString == "time_seed")
						m_type = SEED_TYPE_TIME_SEED;
					else if (seedTypeString == "random")
						m_type = SEED_TYPE_RANDOM;
					else if (seedTypeString == "hourly")
						m_type = SEED_TYPE_HOURLY;
					else if (seedTypeString == "daily")
						m_type = SEED_TYPE_DAILY;
					else if (seedTypeString == "weekly")
						m_type = SEED_TYPE_WEEKLY;
					else if (seedTypeString == "monthly")
						m_type = SEED_TYPE_MONTHLY;
					else if (seedTypeString == "yearly")
						m_type = SEED_TYPE_RANDOM;
					else
						TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid seed type.", aSource->GetIdentifier());
				}
			}

			void	
			ToStream(
				IWriter*					aStream) const
			{
				aStream->WritePOD(m_type);

				if(m_type == SEED_TYPE_CONSTANT)
					aStream->WriteUInt(m_constant);
			}
			
			bool	
			FromStream(
				IReader*					aStream)
			{
				if (!aStream->ReadPOD(m_type))
					return false;

				if(m_type == SEED_TYPE_CONSTANT)
				{
					if (!aStream->ReadUInt(m_constant))
						return false;
				}
				return true;
			}

			bool
			IsOpenWorldValid() const
			{
				switch(m_type)
				{
				case SEED_TYPE_CONSTANT:
				case SEED_TYPE_TIME_SEED:
					return true;

				default:
					return false;
				}
			}

			// Public data
			SeedType			m_type = SEED_TYPE_CONSTANT;
			uint32_t			m_constant = 0;
		};

		struct EntitySpawn
		{
			void	
			ToStream(
				IWriter*					aStream) const
			{
				aStream->WriteInt(m_x);
				aStream->WriteInt(m_y);
				aStream->WriteUInt(m_mapEntitySpawnId);
				aStream->WriteUInt(m_entityId);
			}
			
			bool	
			FromStream(
				IReader*					aStream)
			{
				if (!aStream->ReadInt(m_x))
					return false;
				if (!aStream->ReadInt(m_y))
					return false;
				if (!aStream->ReadUInt(m_mapEntitySpawnId))
					return false;
				if (!aStream->ReadUInt(m_entityId))
					return false;
				return true;
			}

			// Public data
			int32_t									m_x = 0;
			int32_t									m_y = 0;
			uint32_t								m_mapEntitySpawnId = 0;
			uint32_t								m_entityId = 0;
		};

		struct PlayerSpawn
		{
			void	
			ToStream(
				IWriter*					aStream) const
			{
				aStream->WriteInt(m_x);
				aStream->WriteInt(m_y);
				aStream->WriteUInt(m_id);
			}
			
			bool	
			FromStream(
				IReader*					aStream)
			{
				if (!aStream->ReadInt(m_x))
					return false;
				if (!aStream->ReadInt(m_y))
					return false;
				if (!aStream->ReadUInt(m_id))
					return false;
				return true;
			}

			// Public data
			int32_t									m_x = 0;
			int32_t									m_y = 0;
			uint32_t								m_id = 0;
		};

		struct Portal
		{
			void	
			ToStream(
				IWriter*					aStream) const
			{
				aStream->WriteInt(m_x);
				aStream->WriteInt(m_y);
				aStream->WriteUInt(m_id);
			}
			
			bool	
			FromStream(
				IReader*					aStream)
			{
				if (!aStream->ReadInt(m_x))
					return false;
				if (!aStream->ReadInt(m_y))
					return false;
				if (!aStream->ReadUInt(m_id))
					return false;
				return true;
			}

			// Public data
			int32_t									m_x = 0;
			int32_t									m_y = 0;
			uint32_t								m_id = 0;
		};

		struct ScriptCondition
		{
			enum Type : uint8_t
			{
				TYPE_NONE,
				TYPE_TRIGGER_NOT_SET, 
				TYPE_TRIGGER_SET,
				TYPE_ENTITY_STATE
			};

			ScriptCondition()
			{

			}

			ScriptCondition(
				const SourceNode*			aSource)
			{
				if(aSource->m_name == "trigger_not_set")
					m_type = TYPE_TRIGGER_NOT_SET;
				else if (aSource->m_name == "trigger_set")
					m_type = TYPE_TRIGGER_SET;
				else if (aSource->m_name == "entity_state")
					m_type = TYPE_ENTITY_STATE;
				else
					TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid script condition.", aSource->m_name.c_str());

				if(m_type == TYPE_TRIGGER_NOT_SET || m_type == TYPE_TRIGGER_SET)
				{
					m_mapTriggerId = aSource->GetId(DataType::ID_MAP_TRIGGER);
				}
				else if(m_type == TYPE_ENTITY_STATE)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "entity_spawn")
						{
							m_mapEntitySpawnId = aChild->GetId(DataType::ID_MAP_ENTITY_SPAWN);
						}
						else if(aChild->m_name == "state")
						{
							m_entityState = EntityState::StringToId(aChild->GetIdentifier());
							TP_VERIFY(m_entityState != EntityState::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid entity state.", aChild->GetIdentifier());
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					});
				}
			}

			void	
			ToStream(
				IWriter*					aStream) const
			{
				aStream->WritePOD(m_type);
				aStream->WriteUInt(m_mapTriggerId);
				aStream->WriteUInt(m_mapEntitySpawnId);
				aStream->WritePOD(m_entityState);
			}
			
			bool	
			FromStream(
				IReader*					aStream)
			{
				if (!aStream->ReadPOD(m_type))
					return false;
				if (!aStream->ReadUInt(m_mapTriggerId))
					return false;
				if (!aStream->ReadUInt(m_mapEntitySpawnId))
					return false;
				if (!aStream->ReadPOD(m_entityState))
					return false;
				return true;
			}

			// Public data
			Type									m_type = TYPE_NONE;

			// TYPE_TRIGGER_NOT_SET, TYPE_TRIGGER_SET
			uint32_t								m_mapTriggerId = 0;

			// TYPE_ENTITY_STATE
			uint32_t								m_mapEntitySpawnId = 0;
			EntityState::Id							m_entityState = EntityState::INVALID_ID;
		};

		struct ScriptCommand
		{	
			enum Type : uint8_t
			{
				TYPE_NONE,
				TYPE_SET_TRIGGER,
				TYPE_CLEAR_TRIGGER
			};

			ScriptCommand()
			{

			}

			ScriptCommand(
				const SourceNode*			aSource)
			{
				if(aSource->m_name == "set_trigger")
					m_type = TYPE_SET_TRIGGER;
				else if(aSource->m_name == "clear_trigger")
					m_type = TYPE_CLEAR_TRIGGER;
				else
					TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid script command.", aSource->m_name.c_str());

				if(m_type == TYPE_SET_TRIGGER || m_type == TYPE_CLEAR_TRIGGER)
					m_mapTriggerId = aSource->GetId(DataType::ID_MAP_TRIGGER);
			}

			void	
			ToStream(
				IWriter*					aStream) const
			{
				aStream->WritePOD(m_type);
				aStream->WriteUInt(m_mapTriggerId);
			}
			
			bool	
			FromStream(
				IReader*					aStream)
			{
				if (!aStream->ReadPOD(m_type))
					return false;
				if (!aStream->ReadUInt(m_mapTriggerId))
					return false;
				return true;
			}

			// Public data
			Type									m_type = TYPE_NONE;

			// TYPE_SET_TRIGGER, TYPE_CLEAR_TRIGGER
			uint32_t								m_mapTriggerId = 0;
		};

		struct Script
		{			
			Script()
			{

			}

			Script(
				const SourceNode*			aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_tag == "condition")
					{
						m_conditions.push_back(ScriptCondition(aChild));
					}
					else if(aChild->m_name == "run")
					{	
						aChild->GetObject()->ForEachChild([&](
							const SourceNode* aCommand)
						{
							m_commands.push_back(ScriptCommand(aCommand));
						});
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}						
				});
			}

			void	
			ToStream(
				IWriter*					aStream) const
			{
				aStream->WriteObjects(m_conditions);
				aStream->WriteObjects(m_commands);
			}
			
			bool	
			FromStream(
				IReader*					aStream)
			{
				if (!aStream->ReadObjects(m_conditions))
					return false;
				if (!aStream->ReadObjects(m_commands))
					return false;
				return true;
			}

			// Public data
			std::vector<ScriptCondition>			m_conditions;
			std::vector<ScriptCommand>				m_commands;
		};

		struct Generator
		{
			Generator()
			{

			}

			Generator(
				const SourceNode*			aSource)
			{
				MapGenerator::Id mapGeneratorId = MapGenerator::StringToId(aSource->m_name.c_str());
				TP_VERIFY(mapGeneratorId != MapGenerator::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid map generator.", aSource->m_name.c_str());
				m_base.reset(aSource->m_sourceContext->m_mapGeneratorFactory->Create(mapGeneratorId));
				assert(m_base);
				m_base->FromSource(aSource);
			}

			void	
			ToStream(
				IWriter*					aStream) const
			{
				assert(m_base);
				aStream->WritePOD(m_base->m_id);
				m_base->ToStream(aStream);
			}
			
			bool	
			FromStream(
				IReader*					aStream)
			{
				assert(!m_base);
				MapGenerator::Id mapGeneratorId;
				if(!aStream->ReadPOD(mapGeneratorId))
					return false;
				if(!MapGenerator::ValidateId(mapGeneratorId))
					return false;
				m_base.reset(aStream->GetMapGeneratorFactory()->Create(mapGeneratorId));
				assert(m_base);
				if(!m_base->FromStream(aStream))
					return false;
				return true;
			}

			// Public data
			std::unique_ptr<MapGeneratorBase>	m_base;
		};

		enum PointOfInterestType : uint8_t
		{
			INVALID_POINT_OF_INTEREST_TYPE,
			
			POINT_OF_INTEREST_TYPE_QUEST_GIVER
		};

		struct PointOfInterest
		{
			void
			ToStream(
				IWriter* aWriter) const
			{
				aWriter->WritePOD(m_type);
				m_position.ToStream(aWriter);
				aWriter->WriteUInts(m_questIds);
				aWriter->WriteUInt(m_entityId);
				aWriter->WriteUInt(m_dialogueRootId);
			}

			bool
			FromStream(
				IReader* aReader)
			{
				if (!aReader->ReadPOD(m_type))
					return false;
				if (!m_position.FromStream(aReader))
					return false;
				if (!aReader->ReadUInts(m_questIds))
					return false;
				if (!aReader->ReadUInt(m_entityId))
					return false;
				if (!aReader->ReadUInt(m_dialogueRootId))
					return false;
				return true;
			}

			// Public data
			PointOfInterestType		m_type = INVALID_POINT_OF_INTEREST_TYPE;
			Vec2					m_position;
			uint32_t				m_dialogueRootId = 0;
			uint32_t				m_entityId = 0;
			std::vector<uint32_t>	m_questIds;
		};

							MapData();
							MapData(
								const SourceNode*		aSource);
							~MapData();

		void				Build(
								const Manifest*			aManifest,
								const AutoDoodads*		aAutoDoodads,
								nwork::Queue*			aWorkQueue);
		void				ConstructMapPathData(
								const Manifest*			aManifest,
								nwork::Queue*			aWorkQueue);
		void				ConstructMapRouteData(
								const Manifest*			aManifest,
								nwork::Queue*			aWorkQueue);
		void				ToStream(
								IWriter*				aStream) const;
		bool				FromStream(
								IReader*				aStream);
		void				PrepareRuntime(
								uint8_t					aRuntime,
								const Manifest*			aManifest);
		bool				IsTileWalkable(
								int32_t					aX,
								int32_t					aY) const;
		bool				IsTileIndoor(
								int32_t					aX,
								int32_t					aY) const;
		int32_t				TraceWalkableTiles(
								const Vec2&				aPosition,
								const Vec2&				aDirection,
								int32_t					aMaxDistance) const;
		uint8_t				GetElevation(
								int32_t					aX,
								int32_t					aY) const;
		bool				DoesTileBlockLineOfSight(
								int32_t					aX,
								int32_t					aY) const;
		bool				IsTileAlwaysObscured(
								int32_t					aX,
								int32_t					aY) const;
		bool				DoesNeighborTileBlockLineOfSight(
								int32_t					aX,
								int32_t					aY) const;
		void				CopyFrom(
								const MapData*			aMapData);
		uint32_t			GetTile(
								const Vec2&				aPosition) const;
		void				WriteDebugTileMapPNG(
								const Manifest*			aManifest,
								const char*				aPath) const;
		uint32_t			GetDoodad(
								const Vec2&				aPosition) const;
		uint32_t			GetWall(
								const Vec2&				aPosition) const;
		const char*			GetStaticPositionToolTip(
								const Vec2&				aPosition) const;
		const PlayerSpawn*	GetPlayerSpawn(
								uint32_t				aMapPlayerSpawnId) const;
		
		// Public data
		MapType::Id									m_type;
		MapType::ResetMode							m_resetMode;
		MapInfo										m_mapInfo;
		std::string									m_imageOutputPath;
		int32_t										m_x;
		int32_t										m_y;
		int32_t										m_width;
		int32_t										m_height;
		uint32_t*									m_tileMap;
		uint8_t*									m_elevationMap;
		Seed										m_seed;
		std::vector<EntitySpawn>					m_entitySpawns;
		std::vector<PlayerSpawn>					m_playerSpawns;
		std::vector<Portal>							m_portals;
		std::vector<std::unique_ptr<Script>>		m_scripts;
		uint32_t*									m_walkableBits;
		uint32_t*									m_blockLineOfSightBits;
		uint32_t*									m_alwaysObscuredBits;
		std::unique_ptr<MapPathData>				m_mapPathData;
		std::unique_ptr<Generator>					m_generator;
		std::unique_ptr<WorldInfoMap>				m_worldInfoMap;
		std::unique_ptr<MapCovers>					m_mapCovers;
		std::unique_ptr<MapRouteData>				m_mapRouteData;
		std::unique_ptr<PVP>						m_pvp;
		std::vector<uint32_t>						m_realmBalanceIds;
		std::vector<PointOfInterest>				m_pointsOfInterest;
	
		typedef std::unordered_map<Vec2, std::string, Vec2::Hasher> StaticPositionToolTipTable;
		
		StaticPositionToolTipTable					m_staticPositionToolTips;

		typedef std::unordered_map<Vec2, uint32_t, Vec2::Hasher> ObjectTable;

		ObjectTable									m_doodads;
		ObjectTable									m_walls;

		struct SourceLayer
		{
			struct RGB 
			{
				uint8_t								m_r = 0;
				uint8_t								m_g = 0;
				uint8_t								m_b = 0;				
			};

			SourceLayer()
				: m_x(0)
				, m_y(0)
				, m_width(0)
				, m_height(0)
				, m_rgb(NULL)
				, m_mask(NULL)
				, m_cover(false)
			{

			}

			~SourceLayer()
			{
				if(m_rgb != NULL)
					delete [] m_rgb;

				if(m_mask != NULL)
					delete [] m_mask;
			}

			std::vector<uint32_t>					m_palettes;
			RGB*									m_rgb;
			uint8_t*								m_mask;
			int32_t									m_x;
			int32_t									m_y;
			int32_t									m_width;
			int32_t									m_height;
			bool									m_cover;
			DataErrorHandling::DebugInfo			m_debugInfo;
		};

		std::vector<std::unique_ptr<SourceLayer>>	m_sourceLayers;

	private:

		void		_InitLayers(
						const SourceNode*		aLayersArray);
		void		_InitBits(
						const Manifest*			aManifest);
		void		_InitZonePositionQueryTable(
						const Manifest*			aManifest);
		uint32_t	_GetTile(
						int32_t					aX,
						int32_t					aY);
	};

}