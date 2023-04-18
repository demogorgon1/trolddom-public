#pragma once

#include <kpublic/IWriter.h>

namespace kpublic
{

	class MemoryWriter
		: public IWriter
	{
	public:
					MemoryWriter(
						std::vector<uint8_t>&	aBuffer);
		virtual		~MemoryWriter();

		// IWriter implementation
		void		Write(
						const void*				aBuffer,
						size_t					aBufferSize) override;

	private:

		std::vector<uint8_t>&	m_buffer;
	};

}