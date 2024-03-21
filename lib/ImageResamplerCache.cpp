#include "Pcheader.h"

#include <tpublic/ImageResamplerCache.h>

namespace tpublic
{

	const ImageResampler* 
	ImageResamplerCache::Get(
		const Key&							aKey) const
	{
		Table::const_iterator i = m_table.find(aKey);
		if(i != m_table.cend())
			return i->second.get();
		return NULL;
	}

	void						
	ImageResamplerCache::Insert(
		const Key&							aKey,
		std::unique_ptr<ImageResampler>&	aImageResampler)
	{
		assert(!m_table.contains(aKey));
		m_table[aKey] = std::move(aImageResampler);
	}

}