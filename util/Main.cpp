#include "Pcheader.h"

int
main(
	int		aNumArgs,
	char**	aArgs)
{
	kaos_public::DataErrorHandling::SetErrorCallback([](
		const char* aErrorMessage)
	{
		fprintf(stderr, "Error: %s\n", aErrorMessage);
		exit(EXIT_FAILURE);
	});

	kaos_public::Manifest manifest;

	kaos_public::Compiler compiler(&manifest);

	compiler.Parse("../../data");
	compiler.Build("../../data/_ids.txt", ".");

	manifest.Verify();

	return EXIT_SUCCESS;
}