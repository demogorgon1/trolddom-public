#include "Pcheader.h"

#include <tpublic/Data/Map.h>
#include <tpublic/Data/Tag.h>

#include <tpublic/AuraEffectFactory.h>
#include <tpublic/Compiler.h>
#include <tpublic/ComponentManager.h>
#include <tpublic/Compression.h>
#include <tpublic/DataErrorHandling.h>
#include <tpublic/DirectEffectFactory.h>
#include <tpublic/MemoryWriter.h>
#include <tpublic/ObjectiveTypeFactory.h>
#include <tpublic/Tokenizer.h>

#include "FileWriter.h"
#include "MapImageOutput.h"
#include "PostProcessEntities.h"
#include "SpriteSheetBuilder.h"

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
		_ParseDirectory(aRootPath, aRootPath);

		m_parser.ResolveMacrosAndReferences();
	}

	void	
	Compiler::Build(
		const char*				aPersistentIdTablePath,
		const char*				aDataOutputPath,
		Compression::Level		aCompressionLevel)
	{
		SpriteSheetBuilder spriteSheetBuilder(512);

		m_sourceContext.m_persistentIdTable->Load(aPersistentIdTablePath);

		// Read source
		m_parser.GetRoot()->ForEachChild([&](
			const SourceNode* aNode)
		{
			if(aNode->m_name == "player_components")
			{
				m_manifest->m_playerComponents.FromSource(aNode);
			}
			else if (aNode->m_name == "xp_metrics")
			{
				m_manifest->m_xpMetrics.FromSource(aNode);
			}
			else if (aNode->m_name == "item_metrics")
			{
				m_manifest->m_itemMetrics.FromSource(aNode);
			}
			else if (aNode->m_name == "npc_metrics")
			{
				m_manifest->m_npcMetrics.FromSource(aNode);
			}
			else if (aNode->m_name == "quest_metrics")
			{
				m_manifest->m_questMetrics.FromSource(aNode);
			}
			else if (aNode->m_name == "profession_metrics")
			{
				m_manifest->m_professionMetrics.FromSource(aNode);
			}
			else if (aNode->m_name == "ability_metrics")
			{
				m_manifest->m_abilityMetrics.FromSource(aNode);
			}
			else if(aNode->GetObject()->m_name == "sprites")
			{
				spriteSheetBuilder.AddSprites(aNode);
			}
			else if (aNode->GetObject()->m_name == "word_list")
			{
				WordList::Data::FromSource(aNode, &m_manifest->m_wordList);
			}
			else
			{
				DataType::Id dataType = DataType::StringToId(aNode->m_tag.c_str());
				TP_VERIFY(dataType != DataType::INVALID_ID, aNode->m_debugInfo, "'%s' is not a valid data type.", aNode->m_tag.c_str());

				assert(m_manifest->m_containers[dataType]);
			
				DataBase* base = m_manifest->m_containers[dataType]->GetBaseByName(m_sourceContext.m_persistentIdTable.get(), aNode->m_name.c_str());

				TP_VERIFY(!base->m_defined, aNode->m_debugInfo, "'%s' has already been defined.", aNode->m_name.c_str());

				base->m_debugInfo = aNode->m_debugInfo;

				base->FromSource(aNode);

				base->m_defined = true;
				base->m_componentManager = m_sourceContext.m_componentManager.get();
			}
		});		

		// Prepare word list manifest
		{
			m_manifest->m_wordList.Prepare(m_manifest);

			WordList::QueryCache wordListQueryCache(&m_manifest->m_wordList);

			//WordList::QueryParams queryParams;
			//queryParams.m_mustHaveTags.push_back(m_manifest->GetExistingIdByName<Data::Tag>("adjective"));
			//queryParams.m_mustHaveTags.push_back(m_manifest->GetExistingIdByName<Data::Tag>("evil"));
			//queryParams.Prepare();

			//std::mt19937 random;

			//for(uint32_t i = 0; i < 20; i++)
			//{
			//	const WordList::QueryCache::Query* query = wordListQueryCache.PerformQuery(queryParams);
			//	const WordList::Word* word = query->GetRandomWord(random);
			//	printf("%s\n", word->m_word.c_str());
			//}

			//printf("..\n");
		}

		// Build map data
		{
			spriteSheetBuilder.ExportPreliminaryManifestData(m_sourceContext.m_persistentIdTable.get(), m_manifest);

			m_manifest->GetContainer<tpublic::Data::Map>()->ForEach([&](
				Data::Map* aMap)
			{
				aMap->m_data->Build(m_manifest);

				aMap->m_data->ConstructMapPathData(m_manifest);
			});			
		}

		// Build sprite sheets
		{
			spriteSheetBuilder.Build();
			spriteSheetBuilder.UpdateManifestData();

			std::string spritesPath = aDataOutputPath;
			spritesPath += "/sprites.bin";
			spriteSheetBuilder.ExportSheets(aCompressionLevel, spritesPath.c_str());
		}

		// Generate map image output for debugging purposes
		{
			std::unique_ptr<MapImageOutput> mapImageOutput;

			m_manifest->GetContainer<Data::Map>()->ForEach([&](
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

		// Post process entities
		PostProcessEntities::Run(m_manifest);

		// Export manifest 
		{
			std::vector<uint8_t> uncompressed;
			MemoryWriter writer(uncompressed);
			m_manifest->ToStream(&writer);

			std::vector<uint8_t> compressed;
			Compression::Pack(&uncompressed[0], uncompressed.size(), aCompressionLevel, compressed);

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
		const char*				aRootPath,
		const char*				aPath)
	{
		std::error_code errorCode;
		std::filesystem::directory_iterator it(aPath, errorCode);
		TP_CHECK(!errorCode, "Failed to search directory: %s (%s)", aPath, errorCode.message().c_str());

		for (const std::filesystem::directory_entry& entry : it)
		{
			if (entry.is_regular_file() && entry.path().extension().string() == ".txt")
			{
				if(entry.path().filename().string().c_str()[0] != '_')
				{
					Tokenizer tokenizer(aRootPath, entry.path().string().c_str());
					m_parser.Parse(tokenizer);
				}
			}
			else if(entry.is_directory() && entry.path().filename().string().c_str()[0] != '_')
			{
				_ParseDirectory(aRootPath, entry.path().string().c_str());
			}
		}
	}

}

