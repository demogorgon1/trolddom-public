#pragma once

#include "ComponentBase.h"
#include "ComponentPoolBase.h"
#include "ComponentPoolChunk.h"
#include "ComponentPoolChunkBaseList.h"

namespace tpublic
{

	template <typename _T>
	class ComponentPool
		: public ComponentPoolBase
	{
	public:
		virtual ~ComponentPool() {}

		size_t
		GetFullChunkCount() const
		{
			std::lock_guard lock((std::mutex&)m_lock);

			return m_fullChunks.m_count;
		}

		size_t
		GetPartialChunkCount() const
		{
			std::lock_guard lock((std::mutex&)m_lock);

			return m_partialChunks.m_count;
		}

		// ComponentPoolBase implementation
		ComponentBase*			 
		Allocate() override
		{
			ComponentPoolChunkBase* chunk = NULL;
			uint8_t i = 0;

			{
				std::lock_guard lock(m_lock);

				if (m_partialChunks.m_head != NULL)
				{
					chunk = m_partialChunks.m_head;
					i = chunk->Allocate();
					if (chunk->IsFull())
					{
						m_partialChunks.RemoveChunk(chunk);
						m_fullChunks.AddChunk(chunk);
					}
				}
				else
				{
					chunk = new ComponentPoolChunk<_T>();
					i = chunk->Allocate();
					m_partialChunks.AddChunk(chunk);
				}
			}

			ComponentBase* componentBase = chunk->GetComponentBase(i);
			
			componentBase->InitAllocation(_T::ID, chunk, i);
			componentBase->Cast<_T>()->Reset();
			return componentBase;
		}
		
		void					
		Release(
			ComponentBase*			aComponentBase) override
		{
			assert(aComponentBase->GetComponentId() == _T::ID);

			ComponentPoolChunkBase* chunk = aComponentBase->GetComponentPoolChunkBase();
			uint8_t i = aComponentBase->GetComponentPoolChunkIndex();

			assert(chunk != NULL);

			std::lock_guard lock(m_lock);

			bool wasFull = chunk->IsFull();

			chunk->Release(i);

			if(chunk->IsEmpty())
			{
				assert(!wasFull);
				m_partialChunks.RemoveChunk(chunk);
				delete chunk;
			}
			else if(wasFull)
			{
				m_fullChunks.RemoveChunk(chunk);
				m_partialChunks.AddChunk(chunk);
			}
		}

		void			
		Validate() const override
		{
			std::lock_guard lock((std::mutex&)m_lock);

			m_fullChunks.Validate(true);
			m_partialChunks.Validate(false);
		}

	private:		

		std::mutex						m_lock;

		ComponentPoolChunkBaseList		m_fullChunks;
		ComponentPoolChunkBaseList		m_partialChunks;
	};

}