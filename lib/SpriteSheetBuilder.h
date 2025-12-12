#pragma once

#include <tpublic/Data/Sprite.h>

#include <tpublic/Image.h>
#include <tpublic/Manifest.h>
#include <tpublic/SourceNode.h>
#include <tpublic/SpriteInfo.h>

namespace tpublic
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
					const SourceNode*		aSource);
		void	AddBaseSprites(	
					Manifest*				aManifest,
					const char*				aPath);
		void	Build();
		void	ExportPreliminaryManifestData(
					PersistentIdTable*		aPersistentIdTable,
					Manifest*				aManifest);
		void	UpdateManifestData();
		void	ExportSheets(
					Compression::Level		aCompressionLevel,
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
				, m_index(0)
				, m_data(NULL)
				, m_altGreyscale(NULL)
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
			uint32_t						m_index;
			std::vector<uint32_t>			m_tags;
			Data::Sprite*					m_data;
			const Sprite*					m_altGreyscale;
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

		uint32_t							m_nextUnnamedIndex;

		Sprite*	_CreateSprite(
					const SourceNode*	aNode,
					const char*			aName,
					uint32_t			aSize);
		Sprite*	_GetSprite(
					const char*			aName);
		Sheet*	_CreateSheet(
					uint32_t			aWidth,
					uint32_t			aHeight);
		void	_InsertSprite(
					Sprite*				aSprite);
	};

}