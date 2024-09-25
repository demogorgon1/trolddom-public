#include "Pcheader.h"

#include <tpublic/ComponentPoolChunkBase.h>
#include <tpublic/ComponentPoolChunkBaseList.h>

namespace tpublic
{

	ComponentPoolChunkBaseList::ComponentPoolChunkBaseList()
	{

	}

	ComponentPoolChunkBaseList::~ComponentPoolChunkBaseList()
	{
		while (m_head != NULL)
		{
			assert(m_count > 0);
			ComponentPoolChunkBase* next = m_head->m_next;
			delete m_head;
			m_head = next;
			m_count--;
		}
		assert(m_count == 0);
	}

	void
	ComponentPoolChunkBaseList::AddChunk(
		ComponentPoolChunkBase* aChunk)
	{
		assert(aChunk->m_next == NULL);
		assert(aChunk->m_prev == NULL);
		m_count++;

		aChunk->m_prev = m_tail;

		if (m_head == NULL)
			m_head = aChunk;
		else
			m_tail->m_next = aChunk;

		m_tail = aChunk;
	}

	void
	ComponentPoolChunkBaseList::RemoveChunk(
		ComponentPoolChunkBase* aChunk)
	{
		assert(m_count > 0);
		m_count--;

		if (aChunk->m_next != NULL)
			aChunk->m_next->m_prev = aChunk->m_prev;
		else
			m_tail = aChunk->m_prev;

		if (aChunk->m_prev != NULL)
			aChunk->m_prev->m_next = aChunk->m_next;
		else
			m_head = aChunk->m_next;

		aChunk->m_next = NULL;
		aChunk->m_prev = NULL;
	}

	bool
	ComponentPoolChunkBaseList::HasChunk(
		ComponentPoolChunkBase* aChunk) const
	{
		for (const ComponentPoolChunkBase* t = m_head; t != NULL; t = t->m_next)
		{
			if(t == aChunk)
				return true;
		}
		return false;
	}

	void
	ComponentPoolChunkBaseList::ValidateHasChunk(
		ComponentPoolChunkBase* aChunk) const
	{
		assert(HasChunk(aChunk));
	}

	void	
	ComponentPoolChunkBaseList::Validate(
		bool					aFullExpected) const
	{
		size_t count = 0;

		for(const ComponentPoolChunkBase* t = m_head; t != NULL; t = t->m_next)
		{
			t->Validate();

			if(aFullExpected)
			{
				assert(t->m_freeListSize == 0);
				assert(t->m_realCount == ComponentPoolChunkBase::SIZE);
				assert(t->m_size == ComponentPoolChunkBase::SIZE);
			}
			else
			{
				assert(t->m_realCount < ComponentPoolChunkBase::SIZE);
			}

			count++;
		}

		assert(count == m_count);
	}

}