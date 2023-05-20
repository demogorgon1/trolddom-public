#pragma once

namespace tpublic
{

	namespace Compression
	{

		enum Level
		{
			LEVEL_FAST,
			LEVEL_BEST
		};

		void		Pack(
						const void*				aBuffer,
						size_t					aBufferSize,
						Level					aLevel,
						std::vector<uint8_t>&	aOut);
		bool		Unpack(
						const void*				aBuffer,
						size_t					aBufferSize,
						std::vector<uint8_t>&	aOut);

	}

}