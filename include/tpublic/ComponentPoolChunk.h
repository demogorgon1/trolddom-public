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

		// Public data
		_T								m_components[SIZE];
	};

}