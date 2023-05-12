#pragma once

namespace tpublic
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