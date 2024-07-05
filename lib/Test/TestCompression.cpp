#include "../Pcheader.h"

#include <tpublic/Compression.h>
#include <tpublic/DataErrorHandling.h>

namespace tpublic::Test
{

	void
	TestCompression()
	{
		std::mt19937 random;
		std::vector<uint8_t> input;

		for(uint32_t i = 0; i < 100000; i++)
			input.push_back((uint8_t)(random() % 64));

		std::vector<uint8_t> compressed;
		Compression::Pack(&input[0], input.size(), Compression::LEVEL_BEST, compressed);

		std::vector<uint8_t> uncompressed;
		bool ok = Compression::Unpack(&compressed[0], compressed.size(), uncompressed);
		TP_TEST(ok);
		TP_TEST(uncompressed == input);
	}

}