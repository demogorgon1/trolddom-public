#pragma once

#include "ComponentPoolChunkBase.h"

namespace tpublic
{

	template <typename _T>
	class ComponentPoolChunk
		: public ComponentPoolChunkBase
	{
	public:
		virtual ~ComponentPoolChunk() {} 

		// ComponentPoolChunkBase implementation
		ComponentBase* 
		GetComponentBase(
			uint8_t				aIndex) override
		{
			return &m_components[aIndex];
		}
		
		void			
		Validate() const override
		{
			assert(m_size <= SIZE);

			for(uint8_t i = 0; i < m_freeListSize; i++)
			{
				uint8_t j = m_freeList[i];

				assert(j < m_size);
				assert(m_components[j].GetComponentId() == _T::ID);
			}
		}

		// Public data
		_T								m_components[SIZE];
	};

}