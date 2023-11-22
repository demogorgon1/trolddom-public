#pragma once

#include "Compression.h"
#include "Manifest.h"
#include "Parser.h"
#include "PersistentIdTable.h"

namespace tpublic
{

	class Compiler
	{
	public:
				Compiler(
					Manifest*			aManifest);
				~Compiler();

		void	Parse(
					const char*			aRootPath);
		void	Build(
					const char*			aPersistentIdTablePath,
					const char*			aDataOutputPath,
					Compression::Level	aCompressionLevel);

	private:

		Manifest*							m_manifest;
		Parser								m_parser;
		SourceContext						m_sourceContext;

		void	_ParseDirectory(
					const char*			aRootPath,
					const char*			aPath);
	};

}