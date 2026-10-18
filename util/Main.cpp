#include "Pcheader.h"

#include <tpublic/Compiler.h>

#include "ExportMapXCF.h"

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
				else if (arg == "-export_map_xcf" && i + 1 < aNumArgs)
					m_exportMapXCFPaths.push_back(aArgs[++i]);
				else if (arg == "-check")
					m_check = true;
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
		std::vector<std::string>	m_exportMapXCFPaths;
		bool						m_check = false;
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

	if(!commandLine.m_exportMapXCFPaths.empty())
	{
		tpublic_util::ExportMapXCFResult result = tpublic_util::ExportMapXCF(commandLine.m_exportMapXCFPaths, commandLine.m_check);
		fflush(stdout);

		bool failed = false;

		if(result.m_numErrors > 0)
		{
			fprintf(stderr, "Error: %u layer(s) have pixels with alpha other than 0 or 255.\n", result.m_numErrors);
			failed = true;
		}

		if(commandLine.m_check && result.m_numChanged > 0)
		{
			fprintf(stderr, "Error: %u PNG file(s) don't match their .xcf layers.\n", result.m_numChanged);
			failed = true;
		}

		return failed ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	if(commandLine.m_parseRootPaths.empty())
	{
		printf("tpublic-util <input paths> [options]\n");
		printf("tpublic-util -export_map_xcf <path> [-export_map_xcf <path> ...] [-check]\n");
		printf("\n");
		printf("options:\n");
		printf("  -base <path>      Extend existing data. Path should contain files: manifest.bin and sprites.bin.\n");
		printf("  -ids <path>       Path to persistent id table file. Typically you want to use _ids.txt in tpublic repo.\n");
		printf("  -output <path>    Where to write data. Defaults to ./custom.\n");
		printf("  -gen <path>       Output generated source to this path. If not specified, no source will be generated.\n");
		printf("  -export_map_xcf <path>\n");
		printf("                    Export layers of a map .xcf file (or all .xcf files in a directory) to the PNG files\n");
		printf("                    used as map layer or world map sources in .txt files next to it, named\n");
		printf("                    <xcf name>-<layer name>.png. Only changed PNG files are written. Fails if a map layer\n");
		printf("                    has pixels with alpha other than 0 or 255. Nothing else is done.\n");
		printf("  -check            With -export_map_xcf: don't write anything, fail if any PNG file doesn't match its layer.\n");
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