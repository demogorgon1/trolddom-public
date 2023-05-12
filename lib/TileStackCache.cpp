#include "Pcheader.h"

#include <kpublic/Manifest.h>
#include <kpublic/TileStackCache.h>

namespace tpublic
{

	TileStackCache::TileStackCache(
		Manifest*		aManifest)
		: m_manifest(aManifest)
	{

	}
	
	TileStackCache::~TileStackCache()
	{

	}

	uint32_t		
	TileStackCache::GetSpriteId(
		uint32_t						aBaseTileSpriteId,
		const std::vector<uint32_t>&	aStackedSpriteIds)
	{
		Key key = { aBaseTileSpriteId, aStackedSpriteIds };
		Table::iterator i = m_table.find(key);
		if(i != m_table.end())
			return i->second;		

		Data::Sprite* sprite = m_manifest->m_sprites.CreateUnnamed();

		m_table[key] = sprite->m_id;
		return sprite->m_id;
	}

}