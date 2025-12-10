#include "Pcheader.h"

#include <tpublic/Compiler.h>

namespace tpublic_util
{
	struct CommandLine
	{
		CommandLine(
			int			aNumArgs,
			char**		aArgs)
		{
			for (int i = 1; i < aNumArgs; i++)
			{
				std::string_view arg(aArgs[i]);

				if (arg == "-base" && i + 1 < aNumArgs)
					m_base = aArgs[++i];
				else if (arg == "-ids" && i + 1 < aNumArgs)
					m_persistentIdTablePath = aArgs[++i];
				else if (arg == "-output" && i + 1 < aNumArgs)
					m_dataOutputPath = aArgs[++i];
				else if (arg == "-gen" && i + 1 < aNumArgs)
					m_generatedSourceOutputPath = aArgs[++i];
				else if(!arg.empty() && arg[0] != '-')
					m_parseRootPaths.push_back(aArgs[i]);
				else
					TP_CHECK(false, "Invalid command line option: %s", aArgs[i]);
			}

			if(m_dataOutputPath.empty())	
				m_dataOutputPath = "./custom";

			if(m_persistentIdTablePath.empty())
				m_persistentIdTablePath = m_dataOutputPath + "/_ids.txt";
		}

		~CommandLine()
		{
		
		}

		// Public data
		std::string					m_base;
		std::vector<std::string>	m_parseRootPaths;
		std::string					m_persistentIdTablePath;
		std::string					m_dataOutputPath;
		std::string					m_generatedSourceOutputPath;
	};
}

int
main(
	int		aNumArgs,
	char**	aArgs)
{
	tpublic::DataErrorHandling::PushErrorCallback([](
		const char* aErrorMessage)
	{
		fprintf(stderr, "Error: %s\n", aErrorMessage);
		exit(EXIT_FAILURE);
	});

	tpublic_util::CommandLine commandLine(aNumArgs, aArgs);

	if(commandLine.m_parseRootPaths.empty())
	{
		printf("tpublic-util <input paths> [options]\n");
		printf("\n");
		printf("options:\n");
		printf("  -base <path>      Extend existing data. Path should contain files: manifest.bin and sprites.bin.\n");
		printf("  -ids <path>       Path to persistent id table file. Typically _ids.txt in tpublic repo.\n");
		printf("  -output <path>    Where to write data. Defaults to current directory.\n");
		printf("  -gen <path>       Output generated source to this path. If not specified, no source will be generated.\n");
		exit(EXIT_FAILURE);
	}

	if (!std::filesystem::exists(commandLine.m_dataOutputPath))
		std::filesystem::create_directories(commandLine.m_dataOutputPath);

	tpublic::Manifest manifest;		

	tpublic::Compiler compiler(&manifest);

	if (!commandLine.m_base.empty())
		compiler.SetBase(commandLine.m_base.c_str());

	compiler.Build(
		commandLine.m_parseRootPaths, 
		commandLine.m_persistentIdTablePath.c_str(), 
		commandLine.m_dataOutputPath.c_str(),
		commandLine.m_generatedSourceOutputPath.c_str(),
		tpublic::Compression::LEVEL_FAST, 
		NULL);

	manifest.Verify();	

	return EXIT_SUCCESS;
}