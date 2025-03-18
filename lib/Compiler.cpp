#include "Pcheader.h"

#include <tpublic/Data/Map.h>
#include <tpublic/Data/Tag.h>

#include <tpublic/AuraEffectFactory.h>
#include <tpublic/AutoDoodads.h>
#include <tpublic/Compiler.h>
#include <tpublic/ComponentManager.h>
#include <tpublic/Compression.h>
#include <tpublic/DataErrorHandling.h>
#include <tpublic/DebugPrintTimer.h>
#include <tpublic/DirectEffectFactory.h>
#include <tpublic/Document.h>
#include <tpublic/MemoryWriter.h>
#include <tpublic/ObjectiveTypeFactory.h>
#include <tpublic/PerfTimer.h>
#include <tpublic/Tokenizer.h>

#include "FileWriter.h"
#include "GenerationJob.h"
#include "JSONManifest.h"
#include "MapImageOutput.h"
#include "PostProcessAbilities.h"
#include "PostProcessDoodads.h"
#include "PostProcessEntities.h"
#include "PostProcessItems.h"
#include "PostProcessSprites.h"
#include "PostProcessWordGenerators.h"
#include "SoundDataBuilder.h"
#include "SpriteSheetBuilder.h"

namespace tpublic
{

	Compiler::Compiler(
		Manifest*				aManifest)
		: m_manifest(aManifest)
		, m_parser(&m_sourceContext)
		, m_buildErrorCount(0)
	{

	}

    Compiler::~Compiler()
	{

	}

	void	
	Compiler::Build(
		const std::vector<std::string>&		aParseRootPaths,
		const char*							aPersistentIdTablePath,
		const char*							aDataOutputPath,
		const char*							aGeneratedSourceOutputPath,
		Compression::Level					aCompressionLevel)
	{
		uint32_t buildFingerprint = _GetInputFingerprint(aParseRootPaths);
		uint32_t currentBuildFingerprint = _GetCurrentBuildFingerprint(aDataOutputPath);
		if(buildFingerprint == currentBuildFingerprint)
			return; // No changes, no need to do anything

		for(const std::string& parseRootPath : aParseRootPaths)
		{
			// Recursively parse all .txt files in root path
			_ParseDirectory(parseRootPath.c_str(), parseRootPath.c_str());

			{
				DataErrorHandling::ScopedErrorCallback scopedErrorCallback([&](
					const char* aString)
				{
					throw BuildError{ aString };
				});

				try
				{
					m_parser.ResolveMacrosAndReferences();
					m_parser.ResolveContextTags();
					m_parser.ResolveEmbeddedDataObjects();
				}
				catch (BuildError& e)
				{
					_OnBuildError(e);
					break;
				}
			}
		}

		nwork::Queue workQueue;
		nwork::ThreadPool threadPool(&workQueue);

		SpriteSheetBuilder spriteSheetBuilder(1024);
		std::vector<std::unique_ptr<GenerationJob>> generationJobs;

		m_sourceContext.m_persistentIdTable->Load(aPersistentIdTablePath);

		// Read source
		m_parser.GetRoot()->ForEachChild([&](
			const SourceNode* aNode)
		{
			DataErrorHandling::ScopedErrorCallback scopedErrorCallback([&](
				const char* aString)
			{
				throw BuildError{ aString };
			});

			try
			{
				_ProcessNode(&spriteSheetBuilder, &generationJobs, aNode);
			}
			catch(BuildError& e)
			{
				_OnBuildError(e);
			}
		});		

		// Process data source
		_ProcessDataQueue();

		TP_CHECK(m_buildErrorCount == 0, "%u build errors.", m_buildErrorCount);

		// Prepare word list manifest
		{
			m_manifest->m_wordList.Prepare(m_manifest);
		}

		// Validate/prune persistent ids and build map data
		{
			spriteSheetBuilder.ExportPreliminaryManifestData(m_sourceContext.m_persistentIdTable.get(), m_manifest);			

			m_sourceContext.m_persistentIdTable->ValidateAndPrune([&](
				const char*							aDataTypeString,
				const char*							aName, 
				const DataErrorHandling::DebugInfo& aDebugInfo)
			{
				_OnBuildError({ Helpers::Format("[%s:%u] Undefined %s '%s' referenced.", aDebugInfo.m_file.c_str(), aDebugInfo.m_line, aDataTypeString, aName) });
			});

			TP_CHECK(m_buildErrorCount == 0, "%u validation errors.", m_buildErrorCount);

			AutoDoodads autoDoodads(m_manifest);

			m_manifest->GetContainer<tpublic::Data::Map>()->ForEach([&](
				Data::Map* aMap)
			{
				printf("Building map '%s'...\n", aMap->m_name.c_str());

				aMap->m_data->Build(m_manifest, &autoDoodads, &workQueue);

				aMap->m_data->ConstructMapPathData(m_manifest, &workQueue);
				aMap->m_data->ConstructMapRouteData(m_manifest, &workQueue);
				return true;
			});			
		}

		// Build sprite sheets
		{
			DebugPrintTimer timer("build sprite sheets");

			spriteSheetBuilder.Build();
			spriteSheetBuilder.UpdateManifestData();

			std::string spritesPath = aDataOutputPath;
			spritesPath += "/sprites.bin";
			spriteSheetBuilder.ExportSheets(aCompressionLevel, spritesPath.c_str());
		}

		// Build sound data
		{
			DebugPrintTimer timer("build sound data");

			std::string soundsPath = aDataOutputPath;
			soundsPath += "/sounds.bin";

			std::string cachePath = aDataOutputPath;
			cachePath += "/cache/opus";

			std::string toolsPath = aDataOutputPath;
			toolsPath += "/../extra/tools";

			SoundDataBuilder soundDataBuilder;
			soundDataBuilder.Build(m_manifest, soundsPath.c_str(), cachePath.c_str(), toolsPath.c_str());
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
				return true;
			});			
		}

		// Post process stuff
		{
			DebugPrintTimer timer("post process data");

			PostProcessEntities::Run(m_manifest);
			PostProcessWordGenerators::Run(m_manifest);
			PostProcessAbilities::Run(m_manifest);
			PostProcessSprites::Run(m_manifest);
			PostProcessItems::Run(m_manifest);
			PostProcessDoodads::Run(m_manifest);
		}

		// Run generation jobs
		for(std::unique_ptr<GenerationJob>& generationJob : generationJobs)
			generationJob->Run(m_manifest, aGeneratedSourceOutputPath);

		// Export JSON manifest
		{
			DebugPrintTimer timer("export json manifest");

			std::string spriteDataPath = aDataOutputPath;
			spriteDataPath += "/sprites.bin";
			JSONManifest jsonManifest(m_manifest, spriteDataPath.c_str());

			std::string path = aDataOutputPath;
			path += "/manifest.json";
			jsonManifest.Export(path.c_str());
		}

		// Export manifest 
		{
			DebugPrintTimer timer("manifest export");

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

		_SaveBuildFingerprint(aDataOutputPath, buildFingerprint);
	}

	//-----------------------------------------------------------------------------------

	uint32_t	
	Compiler::_GetInputFingerprint(
		const std::vector<std::string>& aParseRootPaths)
	{
		Hash::CheckSum checkSum;

		std::vector<std::string> directories = aParseRootPaths;

		while(directories.size() > 0)
		{
			std::string directory = std::move(directories[directories.size() - 1]);
			directories.pop_back();

			std::error_code errorCode;
			std::filesystem::directory_iterator it(directory.c_str(), errorCode);
			TP_CHECK(!errorCode, "Failed to search directory: %s (%s)", directory.c_str(), errorCode.message().c_str());

			for (const std::filesystem::directory_entry& entry : it)
			{
				std::string path = entry.path().string().c_str();

				if (entry.is_regular_file())
				{
					uint64_t timeStamp = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(entry.last_write_time().time_since_epoch()).count();
					size_t fileSize = (size_t)entry.file_size();

					checkSum.AddString(path.c_str());
					checkSum.AddPOD(fileSize);
					checkSum.AddPOD(timeStamp);
				}
				else if(entry.is_directory())
				{
					directories.push_back(path);
				}
			}
		}

		return checkSum.m_hash;
	}

	uint32_t	
	Compiler::_GetCurrentBuildFingerprint(
		const char*				aDataOutputPath)
	{
		std::string buildFingerprintFilePath = std::string(aDataOutputPath) + "/build-fingerprint.txt";
		std::vector<std::string> lines;
		if(!Helpers::LoadTextFile(buildFingerprintFilePath.c_str(), lines) || lines.size() == 0)
			return 0;

		uint32_t value = 0;
		if(sscanf(lines[0].c_str(), "%u", &value) != 1)
			return 0;

		return value;
	}

	void		
	Compiler::_SaveBuildFingerprint(
		const char*				aDataOutputPath,
		uint32_t				aBuildFingerprint)
	{
		std::string buildFingerprintFilePath = std::string(aDataOutputPath) + "/build-fingerprint.txt";
		FileWriter fileWriter(buildFingerprintFilePath.c_str());
		fileWriter.PrintF("%u\r\n", aBuildFingerprint);
	}

	void	
	Compiler::_OnBuildError(
		const BuildError&		aBuildError)
	{
		printf("\x1B[31mERROR: %s\x1B[37m\n", aBuildError.m_string.c_str());
		
		m_buildErrorCount++;
	}

	void	
	Compiler::_ParseDirectory(
		const char*				aRootPath,
		const char*				aPath)
	{
		std::error_code errorCode;
		std::filesystem::directory_iterator it(aPath, errorCode);
		TP_CHECK(!errorCode, "Failed to search directory: %s (%s)", aPath, errorCode.message().c_str());

		PerfTimer timer;

		for (const std::filesystem::directory_entry& entry : it)
		{
			std::string path = entry.path().string().c_str();
			std::replace(path.begin(), path.end(), '\\', '/');

			if (entry.is_regular_file() && entry.path().extension().string() == ".txt")
			{
				if(entry.path().filename().string().c_str()[0] != '_')
				{
					DataErrorHandling::ScopedErrorCallback scopedErrorCallback([&](
						const char* aString)
					{
						throw BuildError{ aString };
					});

					try
					{
						Tokenizer tokenizer(aRootPath, entry.path().string().c_str());
						m_parser.Parse(tokenizer);
					}
					catch(BuildError& e)
					{
						_OnBuildError(e);
					}
				}
			}
			else if(entry.is_directory() && entry.path().filename().string().c_str()[0] != '_')
			{
				_ParseDirectory(aRootPath, path.c_str());
			}
		}

		uint64_t elapsed = timer.GetElapsedMilliseconds();		
		printf("'%s' parsed in %zu ms.\n", aPath, elapsed);
	}

	void	
	Compiler::_ProcessNode(
		SpriteSheetBuilder*								aSpriteSheetBuilder,
		std::vector<std::unique_ptr<GenerationJob>>*	aGenerationJobs,
		const SourceNode*								aNode)
	{
		if (aNode->m_name == "player_components")
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
		else if (aNode->m_name == "misc_metrics")
		{
			m_manifest->m_miscMetrics.FromSource(aNode);
		}
		else if (aNode->m_name == "reputation_metrics")
		{
			m_manifest->m_reputationMetrics.FromSource(aNode);
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
		else if (aNode->m_name == "worship_metrics")
		{
			m_manifest->m_worshipMetrics.FromSource(aNode);
		}
		else if (aNode->m_name == "default_sound_effects")
		{
			m_manifest->m_defaultSoundEffects.FromSource(aNode);
		}
		else if (aNode->m_name == "tile_layering")
		{
			m_manifest->m_tileLayering.FromSource(aNode);
		}
		else if (aNode->m_name == "sprites")
		{
			aSpriteSheetBuilder->AddSprites(aNode);
		}
		else if (aNode->m_name == "word_list")
		{
			WordList::Data::FromSource(aNode, &m_manifest->m_wordList);
		}
		else if (aNode->m_tag == "generation_job")
		{
			aGenerationJobs->push_back(std::make_unique<GenerationJob>(aNode));
		}
		else if(aNode->m_name == "changelog")
		{
			m_manifest->m_changelog = std::make_unique<Document>(aNode);
		}
		else if(aNode->m_name == "base_tile_border_pattern_sprite")
		{
			m_manifest->m_baseTileBorderPatternSpriteId = aNode->GetId(DataType::ID_SPRITE);
		}
		else if(aNode->IsAnonymousObject())
		{
			aNode->ForEachChild([&](
				const SourceNode* aChild)
			{
				_ProcessNode(aSpriteSheetBuilder, aGenerationJobs, aChild);
			});
		}
		else
		{
			DataType::Id dataType = DataType::StringToId(aNode->m_tag.c_str());
			TP_VERIFY(dataType != DataType::INVALID_ID, aNode->m_debugInfo, "'%s' is not a valid data type.", aNode->m_tag.c_str());			

			assert(m_manifest->m_containers[dataType]);

			std::unique_ptr<DataQueueItem> t = std::make_unique<DataQueueItem>();
			t->m_base = m_manifest->m_containers[dataType]->GetBaseByName(m_sourceContext.m_persistentIdTable.get(), aNode->m_name.c_str());
			t->m_base->m_debugInfo = aNode->m_debugInfo;
			t->m_source = aNode;

			TP_VERIFY(!m_dataSourceTable.contains(t->m_base), aNode->m_debugInfo, "'%s' already defined.", aNode->m_name.c_str());
			m_dataSourceTable[t->m_base] = aNode;

			if(aNode->m_extraAnnotation)
			{
				std::vector<const DataBase*>& dataExtends = m_dataExtendsTable[t->m_base];

				if(aNode->m_extraAnnotation->m_type == SourceNode::TYPE_ARRAY)
				{
					aNode->m_extraAnnotation->ForEachChild([&](
						const SourceNode* aChild)
					{
						dataExtends.push_back(m_manifest->m_containers[dataType]->GetBaseByName(m_sourceContext.m_persistentIdTable.get(), aChild->m_name.c_str()));
					});
				}
				else
				{
					dataExtends.push_back(m_manifest->m_containers[dataType]->GetBaseByName(m_sourceContext.m_persistentIdTable.get(), aNode->m_extraAnnotation->GetIdentifier()));
				}
			}

			m_dataQueue.push_back(std::move(t));
		}
	}

	void
	Compiler::_ProcessDataQueue()
	{
		for(std::unique_ptr<DataQueueItem>& t : m_dataQueue)
		{			
			DataErrorHandling::ScopedErrorCallback scopedErrorCallback([&](
				const char* aString)
			{
				throw BuildError{ aString };
			});

			try
			{
				TP_VERIFY(!t->m_base->m_defined, t->m_base->m_debugInfo, "'%s' has already been defined.", t->m_base->m_name.c_str());

				std::vector<const SourceNode*> sources;
				_GetDataExtends(t->m_base, sources);
				assert(sources.size() > 0);

				for(const SourceNode* source : sources)
					t->m_base->FromSource(source);

				t->m_base->m_defined = true;
				t->m_base->m_componentManager = m_sourceContext.m_componentManager.get();
			}
			catch (BuildError& e)
			{
				_OnBuildError(e);
			}
		}
	}

	void		
	Compiler::_GetDataExtends(
		const DataBase*							aDataBase,
		std::vector<const SourceNode*>&			aOut)
	{	
		const SourceNode* source = NULL;

		{
			DataSourceTable::const_iterator i = m_dataSourceTable.find(aDataBase);
			assert(i != m_dataSourceTable.cend());
			source = i->second;
			TP_VERIFY(Helpers::FindItem(aOut, source) == SIZE_MAX, source->m_debugInfo, "Circular polymorphism.");
		}

		{
			DataExtendsTable::const_iterator i = m_dataExtendsTable.find(aDataBase);
			for (const DataBase* extends : i->second)
				_GetDataExtends(extends, aOut);
		}

		aOut.push_back(source);
	}

}

