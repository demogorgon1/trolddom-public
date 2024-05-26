#include "Pcheader.h"

#include <lodepng/lodepng.h>

#include <tpublic/Data/Class.h>
#include <tpublic/Data/Sprite.h>
#include <tpublic/Data/TalentTree.h>

#include <tpublic/DataErrorHandling.h>
#include <tpublic/Manifest.h>

#include "FileWriter.h"
#include "JSONManifest.h"

namespace tpublic
{

	namespace
	{
		struct PNGEncode
		{
			PNGEncode(
				const unsigned char*	aRGBA,
				uint32_t				aWidth,
				uint32_t				aHeight)
			{
				unsigned int result = lodepng_encode32(&m_data, &m_size, aRGBA, aWidth, aHeight);
				TP_CHECK(result == 0 && m_data != NULL && m_size > 0, "Failed to encode PNG for JSON manifest.");
			}

			~PNGEncode()
			{
				assert(m_data != NULL);
				free(m_data);
			}

			// Public data
			unsigned char*	m_data = NULL;
			size_t			m_size = 0;
		};

	}

	//-------------------------------------------------------

	JSONManifest::JSONManifest(
		const Manifest*			aManifest,
		const char*				aSpriteDataPath)
		: m_manifest(aManifest)
	{
		bool ok = m_spriteData.Load(aSpriteDataPath);
		TP_CHECK(ok, "Failed to load sprite data for JSON manifest: %s", aSpriteDataPath);
	}
	
	JSONManifest::~JSONManifest()
	{

	}

	void	
	JSONManifest::Export(
		const char*				aPath)
	{
		_ExtractClasses();

		FileWriter file(aPath);

		file.PrintF("{\r\n");

		// Classes
		{
			file.PrintF("\"classes\":[\r\n");

			for(size_t i = 0; i < m_classes.size(); i++)
			{
				const std::unique_ptr<ClassExport>& classExport = m_classes[i];

				file.PrintF("{\r\n");
				file.PrintF("\"string\":\"%s\",", classExport->m_data->m_displayName.c_str());
				file.PrintF("\"description\":\"%s\",", classExport->m_data->m_description.c_str());
				file.PrintF("\"sprite\":%zu,", classExport->m_sprite->m_index);
				file.PrintF("\r\n");

				file.PrintF("\"talent_trees\":[\r\n");
				for(size_t j = 0; j < classExport->m_talentTrees.size(); j++)
				{
					const std::unique_ptr<ClassTalentTreeExport>& talentTreeExport = classExport->m_talentTrees[j];

					file.PrintF("{\r\n");
					file.PrintF("\"string\":\"%s\",", talentTreeExport->m_data->m_string.c_str());
					file.PrintF("\"description\":\"%s\"", talentTreeExport->m_data->m_description.c_str());

					file.PrintF("}%s\r\n", j + 1 < classExport->m_talentTrees.size() ? "," : "");
				}
				file.PrintF("]\r\n");

				file.PrintF("}%s\r\n", i + 1 < m_classes.size() ? "," : "");
			}

			file.PrintF("],\r\n");
		}

		// Sprites
		{
			std::sort(m_spriteArray.begin(), m_spriteArray.end(), [&](
				const SpriteExport* aLHS,
				const SpriteExport* aRHS)
			{
				return aLHS->m_index < aRHS->m_index;
			});

			file.PrintF("\"sprites\":[\r\n");

			for(size_t j = 0; j < m_spriteArray.size(); j++)
			{
				const SpriteExport* spriteExport = m_spriteArray[j];

				file.PrintF("\"%s\"", spriteExport->m_pngBase64.c_str());
				file.PrintF("%s\r\n", j + 1 < m_spriteArray.size() ? "," : "");
			}

			file.PrintF("]\r\n");
		}

		file.PrintF("}\r\n");
	}

	//------------------------------------------------------------------------

	void
	JSONManifest::_ExtractClasses()
	{
		m_manifest->GetContainer<Data::Class>()->ForEach([&](
			const Data::Class* aClass) -> bool
		{
			std::unique_ptr<ClassExport> classExport = std::make_unique<ClassExport>();
			classExport->m_data = aClass;
			classExport->m_sprite = _GetSpriteExport(aClass->m_spriteId);

			for(uint32_t talentTreeId : aClass->m_talentTrees)
			{
				std::unique_ptr<ClassTalentTreeExport> talentTreeExport = std::make_unique<ClassTalentTreeExport>();
				talentTreeExport->m_data = m_manifest->GetById<Data::TalentTree>(talentTreeId);
				talentTreeExport->m_icon = _GetSpriteExport(talentTreeExport->m_data->m_iconSpriteId);
				classExport->m_talentTrees.push_back(std::move(talentTreeExport));
			}

			m_classes.push_back(std::move(classExport));
			return true;
		});
	}

	JSONManifest::SpriteExport* 
	JSONManifest::_GetSpriteExport(
		uint32_t		aSpriteId)
	{
		SpriteExportTable::iterator i = m_spriteExportTable.find(aSpriteId);
		if(i != m_spriteExportTable.end())
			return i->second.get();

		SpriteExport* t = new SpriteExport();
		t->m_index = m_spriteExportTable.size();
		m_spriteExportTable[aSpriteId] = std::unique_ptr<SpriteExport>(t);
		m_spriteArray.push_back(t);

		// Load sprite and turn it into a base64-encoded PNG
		{
			const Data::Sprite* sprite = m_manifest->GetById<Data::Sprite>(aSpriteId);
			const SpriteData::Sprite& spriteData = m_spriteData.m_sheets[sprite->m_spriteSheetIndex]->m_sprites[sprite->m_index];
			PNGEncode pngEncode((const unsigned char*)spriteData.m_rgba, spriteData.m_width, spriteData.m_height);
			Base64::Encode(pngEncode.m_data, pngEncode.m_size, t->m_pngBase64);
		}

		return t;
	}


}