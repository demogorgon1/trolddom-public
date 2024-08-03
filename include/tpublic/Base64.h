#pragma once

namespace tpublic
{

	namespace Base64
	{

		bool	EncodeToBuffer(
					const void*				aIn,
					size_t					aInSize,
					char*					aBuffer,
					size_t					aBufferSize);
		void	Encode(
					const void*				aIn,
					size_t					aInSize,
					std::string&			aOut);
		void	Decode(
					const char*				aIn,
					std::vector<uint8_t>&	aOut);

	}


}