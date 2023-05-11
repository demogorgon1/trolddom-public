#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace kpublic
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
		uint32_t									m_defaultTileSpriteId;
		uint32_t									m_defaultPlayerSpawnId;
		std::string									m_imageOutputPath;
		int32_t										m_x;
		int32_t										m_y;
		int32_t										m_width;
		int32_t										m_height;
		uint32_t*									m_tileMap;
		std::vector<EntitySpawn>					m_entitySpawns;
		std::vector<PlayerSpawn>					m_playerSpawns;
		std::vector<Portal>							m_portals;
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