#pragma once

namespace tpublic
{

	namespace Base64
	{

		void	Encode(
					const void*				aIn,
					size_t					aInSize,
					std::string&			aOut);
		void	Decode(
					const char*				aIn,
					std::vector<uint8_t>&	aOut);

	}


}