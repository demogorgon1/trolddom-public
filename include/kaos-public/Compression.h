#pragma once

namespace kaos_public
{

	namespace Compression
	{

		void		Pack(
						const void*				aBuffer,
						size_t					aBufferSize,
						std::vector<uint8_t>&	aOut);
		bool		Unpack(
						const void*				aBuffer,
						size_t					aBufferSize,
						std::vector<uint8_t>&	aOut);

	}

}