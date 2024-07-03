#include "Pcheader.h"

#include <tpublic/ComponentPoolChunkBase.h>

namespace tpublic
{

	bool
	ComponentPoolChunkBase::IsFull() const 
	{
		return m_realCount == SIZE;
	}

	bool
	ComponentPoolChunkBase::IsEmpty() const 
	{
		return m_realCount == 0;
	}

	uint8_t
	ComponentPoolChunkBase::Allocate() 
	{
		assert(m_realCount < SIZE);

		uint8_t i = 0;

		if (m_freeListSize > 0)
		{
			i = m_freeList[m_freeListSize - 1];
			m_freeListSize--;
		}
		else
		{
			assert(m_size < SIZE);
			i = m_size++;
		}

		m_realCount++;

		return i;
	}

	void
	ComponentPoolChunkBase::Release(
		uint8_t				aIndex) 
	{
		assert(m_realCount > 0);

		if (aIndex + 1 == m_size)
		{
			m_size--;
		}
		else
		{
			assert(m_freeListSize < SIZE);
			m_freeList[m_freeListSize++] = aIndex;
		}

		m_realCount--;
	}

}