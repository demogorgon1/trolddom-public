#pragma once

#include "Parser.h"

namespace kaos_public
{

	class Manifest;

	class MapData
	{
	public:
		struct EntitySpawn
		{
			int32_t									m_x;
			int32_t									m_y;
			uint32_t								m_id;
		};

		struct PlayerSpawn
		{
			int32_t									m_x;
			int32_t									m_y;
			uint32_t								m_id;
		};

				MapData(
					const Parser::Node*		aSource);
				~MapData();

		void	Build(
					const Manifest*			aManifest);

		// Public data
		uint32_t									m_defaultTileSpriteId;
		std::string									m_imageOutputPath;
		int32_t										m_x;
		int32_t										m_y;
		int32_t										m_width;
		int32_t										m_height;
		uint32_t*									m_tileMap;
		std::vector<EntitySpawn>					m_entitySpawns;
		std::vector<EntitySpawn>					m_playerSpawns;

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

		void	_InitLayers(
					const Parser::Node*		aLayersArray);
	};

}