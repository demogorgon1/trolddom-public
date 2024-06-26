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
					Manifest*										aManifest);
				~Compiler();

		void	Parse(
					const char*										aRootPath);
		void	Build(
					const char*										aPersistentIdTablePath,
					const char*										aDataOutputPath,
					const char*										aGeneratedSourceOutputPath,
					Compression::Level								aCompressionLevel);

	private:

		Manifest*							m_manifest;
		Parser								m_parser;
		SourceContext						m_sourceContext;		

		void	_ParseDirectory(
					const char*										aRootPath,
					const char*										aPath);
		void	_ProcessNode(
					SpriteSheetBuilder*								aSpriteSheetBuilder,
					std::vector<std::unique_ptr<GenerationJob>>*	aGenerationJobs,
					const SourceNode*								aNode);
	};

}