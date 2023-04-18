#include "Pcheader.h"

int
main(
	int		aNumArgs,
	char**	aArgs)
{
	kpublic::DataErrorHandling::SetErrorCallback([](
		const char* aErrorMessage)
	{
		fprintf(stderr, "Error: %s\n", aErrorMessage);
		exit(EXIT_FAILURE);
	});

	kpublic::Manifest manifest;

	kpublic::Compiler compiler(&manifest);

	compiler.Parse("../../data");
	compiler.Build("../../data/_ids.txt", ".");
	manifest.Verify();	

	return EXIT_SUCCESS;
}