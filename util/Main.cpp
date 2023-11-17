#include "Pcheader.h"

int
main(
	int		aNumArgs,
	char**	aArgs)
{
	tpublic::DataErrorHandling::SetErrorCallback([](
		const char* aErrorMessage)
	{
		fprintf(stderr, "Error: %s\n", aErrorMessage);
		exit(EXIT_FAILURE);
	});

	tpublic::Manifest manifest;

	tpublic::Compiler compiler(&manifest);

	compiler.Parse("../../data");
	compiler.Build("../../data/_ids.txt", ".", tpublic::Compression::LEVEL_FAST);
	manifest.Verify();	

	// TESTING
	{
		const tpublic::Data::Map* map = manifest.m_maps.GetExistingByName("test_dungeon_2");
		assert(map != NULL);
		assert(map->m_data->m_generator);

		for(uint32_t i = 0; i < 5; i++)
		{
			char debugImagePath[256];
			TP_STRING_FORMAT(debugImagePath, sizeof(debugImagePath), "debug-%u.png", i);

			std::unique_ptr<tpublic::MapData> generated;
			map->m_data->m_generator->m_base->Build(&manifest, 1234 + i, map->m_data.get(), debugImagePath, generated);
		}
	}

	return EXIT_SUCCESS;
}