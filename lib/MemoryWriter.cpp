#include "Pcheader.h"

#include "MemoryWriter.h"

namespace tpublic
{

	MemoryWriter::MemoryWriter(
		std::vector<uint8_t>&	aBuffer)
		: m_buffer(aBuffer)
	{
	}

	MemoryWriter::~MemoryWriter()
	{
	}

	//------------------------------------------------------------------------

	void		
	MemoryWriter::Write(
		const void*		aBuffer,
		size_t			aBufferSize) 
	{
		if(aBufferSize > 0)
		{
			size_t offset = m_buffer.size();
			m_buffer.resize(offset + aBufferSize);
			memcpy(&m_buffer[offset], aBuffer, aBufferSize);
		}
	}

}