#pragma once

#include "Compression.h"
#include "Manifest.h"
#include "Parser.h"
#include "PersistentIdTable.h"

namespace tpublic
{

	class GenerationJob;
	class SpriteSheetBuilder;

	class Compiler
	{
	public:
				Compiler(
					Manifest*											aManifest);
				~Compiler();

		void	Build(
					const std::vector<std::string>&						aParseRootPaths,
					const char*											aPersistentIdTablePath,
					const char*											aDataOutputPath,
					const char*											aGeneratedSourceOutputPath,
					Compression::Level									aCompressionLevel);

	private:

		struct BuildError
		{
			std::string											m_string;
		};

		Manifest*												m_manifest;
		Parser													m_parser;
		SourceContext											m_sourceContext;		

		size_t													m_buildErrorCount;

		struct DataQueueItem
		{
			DataBase*											m_base = NULL;
			const SourceNode*									m_source = NULL;
			std::vector<const DataBase*>						m_extends;
		};

		std::vector<std::unique_ptr<DataQueueItem>>				m_dataQueue;

		typedef std::unordered_map<const DataBase*, const SourceNode*> DataSourceTable;
		DataSourceTable											m_dataSourceTable;

		uint32_t	_GetInputFingerprint(
						const std::vector<std::string>&					aParseRootPaths);
		uint32_t	_GetCurrentBuildFingerprint(
						const char*										aDataOutputPath);
		void		_SaveBuildFingerprint(
						const char*										aDataOutputPath,
						uint32_t										aBuildFingerprint);
		void		_OnBuildError(
						const BuildError&								aBuildError);
		void		_ParseDirectory(
						const char*										aRootPath,
						const char*										aPath);
		void		_ProcessNode(
						SpriteSheetBuilder*								aSpriteSheetBuilder,
						std::vector<std::unique_ptr<GenerationJob>>*	aGenerationJobs,
						const SourceNode*								aNode);
		size_t		_ProcessDataQueue();
	};

}