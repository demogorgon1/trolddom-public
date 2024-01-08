#pragma once

#include "Manifest.h"
#include "TaggedData.h"

namespace tpublic
{

	template <typename _T>
	class TaggedDataCache
	{
	public:
		static_assert(_T::TAGGED);

		TaggedDataCache(
			const Manifest*		aManifest)
			: m_taggedData(aManifest, _T::DATA_TYPE)
		{
			aManifest->GetContainer<_T>()->ForEach([&](
				const _T* aData)
			{
				if(aData->m_tagIds.size() > 0)
					m_taggedData.Add(aData->m_id, aData->m_tagIds);
			});
		}

		TaggedData*
		Get() 
		{
			return &m_taggedData;
		}

	private:
		
		TaggedData	m_taggedData;
	};

}