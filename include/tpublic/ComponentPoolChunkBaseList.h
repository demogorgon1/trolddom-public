#pragma once

namespace tpublic
{

	struct ComponentPoolChunkBase;

	struct ComponentPoolChunkBaseList
	{
				ComponentPoolChunkBaseList();
				~ComponentPoolChunkBaseList();

		void	AddChunk(
					ComponentPoolChunkBase* aChunk);
		void	RemoveChunk(
					ComponentPoolChunkBase* aChunk);

		// Public data
		size_t					m_count = 0;
		ComponentPoolChunkBase* m_head = NULL;
		ComponentPoolChunkBase* m_tail = NULL;
	};

}