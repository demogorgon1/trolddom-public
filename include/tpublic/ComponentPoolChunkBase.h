#pragma once

namespace tpublic
{

	class ComponentBase;

	struct ComponentPoolChunkBase
	{
		static const uint8_t SIZE = 64;

		virtual					~ComponentPoolChunkBase() {}

		bool					IsFull() const;
		bool					IsEmpty() const;
		uint8_t					Allocate();
		void					Release(
									uint8_t				aIndex);

		// Virtual interface
		virtual ComponentBase*	GetComponentBase(
									uint8_t				aIndex) = 0;
		virtual void			Validate() const = 0;

		// Public data
		uint8_t							m_realCount = 0;
		uint8_t							m_size = 0;
		uint8_t							m_freeList[SIZE];
		uint8_t							m_freeListSize = 0;
		ComponentPoolChunkBase*			m_next = NULL;
		ComponentPoolChunkBase*			m_prev = NULL;
	};

}