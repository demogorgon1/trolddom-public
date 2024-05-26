#pragma once

#include <tpublic/SpriteData.h>

namespace tpublic
{

	namespace Data
	{
		struct Class;
		struct TalentTree;
	}

	class Manifest;

	class JSONManifest
	{
	public:
				JSONManifest(
					const Manifest*			aManifest,
					const char*				aSpriteDataPath);
				~JSONManifest();

		void	Export(
					const char*				aPath);

	private:

		const Manifest*											m_manifest;
		SpriteData												m_spriteData;

		struct SpriteExport
		{
			const Data::Sprite*									m_data = NULL;
			std::string											m_pngBase64;
			size_t												m_index = 0;
		};

		typedef std::unordered_map<uint32_t, std::unique_ptr<SpriteExport>> SpriteExportTable;
		SpriteExportTable										m_spriteExportTable;
		std::vector<const SpriteExport*>						m_spriteArray;

		struct ClassTalentTreeExport							
		{
			const Data::TalentTree*								m_data = NULL;
			const SpriteExport*									m_icon = NULL;
		};

		struct ClassExport
		{
			const Data::Class*									m_data = NULL;
			const SpriteExport*									m_sprite = NULL;
			std::vector<std::unique_ptr<ClassTalentTreeExport>>	m_talentTrees;
		};

		std::vector<std::unique_ptr<ClassExport>>				m_classes;

		void			_ExtractClasses();
		SpriteExport*	_GetSpriteExport(
							uint32_t		aSpriteId);
	};

}