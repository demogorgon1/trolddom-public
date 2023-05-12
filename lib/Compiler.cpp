#include "Pcheader.h"

#include <tpublic/Compiler.h>
#include <tpublic/ComponentFactory.h>
#include <tpublic/Compression.h>
#include <tpublic/DataErrorHandling.h>
#include <tpublic/TileStackCache.h>

#include "FileWriter.h"
#include "MapImageOutput.h"
#include "MemoryWriter.h"
#include "SpriteSheetBuilder.h"
#include "Tokenizer.h"

namespace tpublic
{

	Compiler::Compiler(
		Manifest*				aManifest)
		: m_manifest(aManifest)
		, m_parser(&m_sourceContext)
	{

	}

    Compiler::~Compiler()
	{

	}

	void	
	Compiler::Parse(
		const char*				aRootPath)
	{
		// Recursively parse all .txt files in root path
		_ParseDirectory(aRootPath);
	}

	void	
	Compiler::Build(
		const char*				aPersistentIdTablePath,
		const char*				aDataOutputPath)
	{
		SpriteSheetBuilder spriteSheetBuilder(512);

		m_sourceContext.m_persistentIdTable->Load(aPersistentIdTablePath);

		// Read source
		m_parser.GetRoot()->ForEachChild([&](
			const Parser::Node* aNode)
		{
			if(aNode->m_name == "player_components")
			{
				m_manifest->m_playerComponents.FromSource(aNode);
			}
			else if(aNode->GetObject()->m_name == "sprites")
			{
				spriteSheetBuilder.AddSprites(aNode);
			}
			else
			{
				DataType::Id dataType = DataType::StringToId(aNode->m_tag.c_str());
				KP_VERIFY(dataType != DataType::INVALID_ID, aNode->m_debugInfo, "'%s' is not a valid data type.", aNode->m_tag.c_str());

				assert(m_manifest->m_containers[dataType] != NULL);
			
				DataBase* base = m_manifest->m_containers[dataType]->GetBaseByName(m_sourceContext.m_persistentIdTable.get(), aNode->m_name.c_str());

				KP_VERIFY(!base->m_defined, aNode->m_debugInfo, "'%s' has already been defined.", aNode->m_name.c_str());

				base->m_debugInfo = aNode->m_debugInfo;

				base->FromSource(aNode);

				base->m_defined = true;
			}
		});		

		// Build map data
		TileStackCache tileStackCache(m_manifest);

		{
			spriteSheetBuilder.ExportPreliminaryManifestData(m_sourceContext.m_persistentIdTable.get(), m_manifest);

			m_manifest->m_maps.ForEach([&](
				Data::Map* aMap)
			{
				aMap->m_data->Build(m_manifest, &tileStackCache);
			});			
		}

		// Build sprite sheets
		{
			spriteSheetBuilder.GenerateStackedTiles(&tileStackCache, m_manifest);
			spriteSheetBuilder.Build();
			spriteSheetBuilder.UpdateManifestData();

			std::string spritesPath = aDataOutputPath;
			spritesPath += "/sprites.bin";
			spriteSheetBuilder.ExportSheets(spritesPath.c_str());
		}

		// Generate map image output for debugging purposes
		{
			std::unique_ptr<MapImageOutput> mapImageOutput;

			m_manifest->m_maps.ForEach([&](
				Data::Map* aMap)
			{
				// Debug image output for this map?
				if(!aMap->m_data->m_imageOutputPath.empty())
				{
					if(!mapImageOutput)
						mapImageOutput = std::make_unique<MapImageOutput>(aDataOutputPath, m_manifest);

					mapImageOutput->Generate(aMap->m_data.get(), aMap->m_data->m_imageOutputPath.c_str());
				}
			});			
		}

		// Export manifest 
		{
			std::vector<uint8_t> uncompressed;
			MemoryWriter writer(uncompressed);
			m_manifest->ToStream(&writer);

			std::vector<uint8_t> compressed;
			Compression::Pack(&uncompressed[0], uncompressed.size(), compressed);

			std::string path = aDataOutputPath;
			path += "/manifest.bin";			
			FileWriter file(path.c_str());
			file.Write(&compressed[0], compressed.size());
		}

		m_sourceContext.m_persistentIdTable->Save();
	}

	//-----------------------------------------------------------------------------------

	void	
	Compiler::_ParseDirectory(
		const char*				aPath)
	{
		std::error_code errorCode;
		std::filesystem::directory_iterator it(aPath, errorCode);
		KP_CHECK(!errorCode, "Failed to search directory: %s (%s)", aPath, errorCode.message().c_str());

		for (const std::filesystem::directory_entry& entry : it)
		{
			if (entry.is_regular_file() && entry.path().extension().string() == ".txt")
			{
				if(entry.path().filename().string().c_str()[0] != '_')
				{
					Tokenizer tokenizer(entry.path().string().c_str());

					m_parser.Parse(tokenizer);
				}
			}
			else if(entry.is_directory())
			{
				_ParseDirectory(entry.path().string().c_str());
			}
		}
	}

}