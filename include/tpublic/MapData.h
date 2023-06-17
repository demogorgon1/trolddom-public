#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "MapType.h"
#include "Parser.h"

namespace tpublic
{

	class Manifest;
	class TileStackCache;

	class MapData
	{
	public:
		struct EntitySpawn
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
				const Parser::Node*			aSource)
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
					m_mapTriggerId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_TRIGGER, aSource->GetIdentifier());
				}
				else if(m_type == TYPE_ENTITY_STATE)
				{
					aSource->ForEachChild([&](
						const Parser::Node* aChild)
					{
						if(aChild->m_name == "entity_spawn")
						{
							m_mapEntitySpawnId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_ENTITY_SPAWN, aChild->GetIdentifier());
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
				const Parser::Node*			aSource)
			{
				if(aSource->m_name == "set_trigger")
					m_type = TYPE_SET_TRIGGER;
				else if(aSource->m_name == "clear_trigger")
					m_type = TYPE_CLEAR_TRIGGER;
				else
					TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid script command.", aSource->m_name.c_str());

				if(m_type == TYPE_SET_TRIGGER || m_type == TYPE_CLEAR_TRIGGER)
					m_mapTriggerId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_TRIGGER, aSource->GetIdentifier());
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
				const Parser::Node*			aSource)
			{
				aSource->ForEachChild([&](
					const Parser::Node* aChild)
				{
					if(aChild->m_tag == "condition")
					{
						m_conditions.push_back(ScriptCondition(aChild));
					}
					else if(aChild->m_name == "run")
					{	
						aChild->GetObject()->ForEachChild([&](
							const Parser::Node* aCommand)
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

				MapData();
				MapData(
					const Parser::Node*		aSource);
				~MapData();

		void	Build(
					const Manifest*			aManifest,
					TileStackCache*			aTileStackCache);
		void	ToStream(
					IWriter*				aStream) const;
		bool	FromStream(
					IReader*				aStream);
		void	PrepareRuntime(
					const Manifest*			aManifest);
		bool	IsTileWalkable(
					int32_t					aX,
					int32_t					aY) const;
		bool	DoesTileblockLineOfSight(
					int32_t					aX,
					int32_t					aY) const;

		// Public data
		MapType::Id									m_type;
		MapType::ResetMode							m_resetMode;
		std::string									m_displayName;
		uint32_t									m_defaultTileSpriteId;
		uint32_t									m_defaultPlayerSpawnId;
		uint32_t									m_defaultExitPortalId;
		uint32_t									m_viewAttenuation;
		uint32_t									m_viewAttenuationBias;
		uint32_t									m_viewHiddenVisibility;
		std::string									m_imageOutputPath;
		int32_t										m_x;
		int32_t										m_y;
		int32_t										m_width;
		int32_t										m_height;
		uint32_t*									m_tileMap;
		std::vector<EntitySpawn>					m_entitySpawns;
		std::vector<PlayerSpawn>					m_playerSpawns;
		std::vector<Portal>							m_portals;
		std::vector<std::unique_ptr<Script>>		m_scripts;
		uint32_t*									m_walkableBits;
		uint32_t*									m_blockLineOfSightBits;

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
			DataErrorHandling::DebugInfo			m_debugInfo;
		};

		std::vector<std::unique_ptr<SourceLayer>>	m_sourceLayers;

	private:

		void		_InitLayers(
						const Parser::Node*		aLayersArray);
		void		_InitBits(
						const Manifest*			aManifest);
		uint32_t	_GetTile(
						int32_t					aX,
						int32_t					aY);
	};

}