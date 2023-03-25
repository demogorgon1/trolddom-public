#pragma once

#include "Manifest.h"
#include "Parser.h"
#include "PersistentIdTable.h"

namespace kaos_public
{

	class Compiler
	{
	public:
				Compiler(
					Manifest*		aManifest);
				~Compiler();

		void	Parse(
					const char*		aRootPath);
		void	Build(
					const char*		aPersistentIdTablePath);

	private:

		Manifest*			m_manifest;
		Parser				m_parser;
		SourceContext		m_sourceContext;

		void	_ParseDirectory(
					const char*		aPath);
	};

}