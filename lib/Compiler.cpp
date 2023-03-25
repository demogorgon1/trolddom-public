#include "Pcheader.h"

#include <kaos-public/Compiler.h>
#include <kaos-public/ComponentFactory.h>
#include <kaos-public/DataErrorHandling.h>

#include "Tokenizer.h"

namespace kaos_public
{

	Compiler::Compiler(
		Manifest*			aManifest)
		: m_manifest(aManifest)
		, m_parser(&m_sourceContext)
	{

	}

    Compiler::~Compiler()
	{

	}

	void	
	Compiler::Parse(
		const char*			aRootPath)
	{
		// Recursively parse all .txt files in root path
		_ParseDirectory(aRootPath);
	}

	void	
	Compiler::Build(
		const char*			aPersistentIdTablePath)
	{
		m_sourceContext.m_persistentIdTable->Load(aPersistentIdTablePath);

		m_parser.GetRoot()->ForEachChild([&](
			const Parser::Node* aNode)
		{
			DataType::Id dataType = DataType::StringToId(aNode->m_tag.c_str());
			KP_VERIFY(dataType != DataType::INVALID_ID, aNode->m_debugInfo, "'%s' is not a valid data type.", aNode->m_tag.c_str());

			assert(m_manifest->m_containers[dataType] != NULL);
			
			DataBase* base = m_manifest->m_containers[dataType]->GetBaseByName(m_sourceContext.m_persistentIdTable.get(), aNode->m_name.c_str());

			KP_VERIFY(!base->m_defined, aNode->m_debugInfo, "'%s' has already been defined.", aNode->m_name.c_str());

			base->m_debugInfo = aNode->m_debugInfo;

			base->FromSource(aNode);

			base->m_defined = true;
		});		

		m_sourceContext.m_persistentIdTable->Save();
	}

	//-----------------------------------------------------------------------------------

	void	
	Compiler::_ParseDirectory(
		const char*			aPath)
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