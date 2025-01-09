#include "Pcheader.h"

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

	tpublic::Manifest manifest;

	tpublic::Compiler compiler(&manifest);

	compiler.Parse("../../data");
	compiler.Build("../../data/_ids.txt", ".", "../../data/generated", tpublic::Compression::LEVEL_FAST);
	manifest.Verify();	

	return EXIT_SUCCESS;
}