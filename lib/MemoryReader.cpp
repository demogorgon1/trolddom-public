#include "Pcheader.h"

#include <tpublic/MemoryReader.h>

namespace tpublic
{

	MemoryReader::MemoryReader(
		const void*			aBuffer,
		size_t				aBufferSize)
		: m_p((const uint8_t*)aBuffer)
		, m_remaining(aBufferSize)
	{

	}
	
	MemoryReader::~MemoryReader()
	{

	}

	//----------------------------------------------------------------------------
	
	bool		
	MemoryReader::IsEnd() const 
	{
		return m_remaining == 0;
	}
	
	size_t		
	MemoryReader::Read(
		void*				aBuffer,
		size_t				aBufferSize) 
	{
		size_t toCopy = Base::Min(aBufferSize, m_remaining);
		memcpy(aBuffer, m_p, toCopy);
		m_p += toCopy;
		m_remaining -= toCopy;
		return toCopy;
	}

	size_t		
	MemoryReader::Peek(
		void*				aBuffer,
		size_t				aBufferSize) const
	{
		size_t toCopy = Base::Min(aBufferSize, m_remaining);
		memcpy(aBuffer, m_p, toCopy);
		return toCopy;
	}
}