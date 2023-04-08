#pragma once

#include <kaos-public/Manifest.h>
#include <kaos-public/Parser.h>
#include <kaos-public/SpriteInfo.h>

#include "Image.h"

namespace kaos_public
{

	class Manifest;
	class PersistentIdTable;

	class SpriteSheetBuilder
	{
	public:
				SpriteSheetBuilder(
					uint32_t				aMinSheetSize);
				~SpriteSheetBuilder();

		void	AddSprites(
					const Parser::Node*		aSource);
		void	Build();
		void	ExportManifestData(
					PersistentIdTable*		aPersistentIdTable,
					Manifest*				aManifest);
		void	ExportSheets(
					const char*				aPath);

	private:

		struct Sprite
		{			
			Sprite(
				const char*					aName,
				uint32_t					aSize)
				: m_name(aName)
				, m_size(aSize)
				, m_sheetOffsetX(0)
				, m_sheetOffsetY(0)
				, m_sheetIndex(0)
			{

			}

			// Public data
			std::string						m_name;
			Image							m_image;
			uint32_t						m_size;
			SpriteInfo						m_info;
			uint32_t						m_sheetIndex;
			uint32_t						m_sheetOffsetX;
			uint32_t						m_sheetOffsetY;
		};

		struct Sheet
		{
			struct EmptySpace
			{
				uint32_t					m_x;
				uint32_t					m_y;
				uint32_t					m_width;
				uint32_t					m_height;
			};

			Sheet(
				uint32_t					aIndex,
				uint32_t					aWidth,
				uint32_t					aHeight)
				: m_index(aIndex)
				, m_width(aWidth)
				, m_height(aHeight)
			{
				m_emptySpaces.push_back({ 0, 0, aWidth, aHeight });
			}

			uint32_t						m_index;
			uint32_t						m_width;
			uint32_t						m_height;
			std::vector<const Sprite*>		m_sprites;
			std::vector<EmptySpace>			m_emptySpaces;
		};

		uint32_t							m_minSheetSize;

		typedef std::unordered_map<std::string, std::unique_ptr<Sprite>> SpriteTable;
		SpriteTable							m_spriteTable;		

		std::vector<std::unique_ptr<Sheet>>	m_sheets;

		Sprite*	_CreateSprite(
					const Parser::Node*	aNode,
					const char*			aName,
					uint32_t			aSize);
		Sheet*	_CreateSheet(
					uint32_t			aWidth,
					uint32_t			aHeight);
		void	_InsertSprite(
					Sprite*				aSprite);
	};

}