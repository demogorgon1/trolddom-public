#pragma once

#include "Manifest.h"
#include "Parser.h"
#include "PersistentIdTable.h"

namespace kpublic
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
					const char*			aDataOutputPath);

	private:

		Manifest*			m_manifest;
		Parser				m_parser;
		SourceContext		m_sourceContext;

		void	_ParseDirectory(
					const char*			aPath);
	};

}