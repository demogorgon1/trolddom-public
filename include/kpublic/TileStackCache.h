#pragma once

#include "Hash.h"

namespace kpublic
{

	class Manifest;

	class TileStackCache
	{
	public:	
		struct Key
		{
			struct Hasher
			{
				uint32_t
				operator()(
					const Key&								aKey) const
				{
					uint64_t x = Hash::Splitmix_64((uint64_t)aKey.m_baseTileSpriteId);
					for (uint32_t y : aKey.m_stackedSpriteIds)
						x = Hash::Splitmix_64((uint64_t)y ^ x);
					return (uint32_t)x;
				}
			};

			bool
			operator==(
				const Key&									aKey) const
			{
				return aKey.m_baseTileSpriteId == m_baseTileSpriteId && aKey.m_stackedSpriteIds == m_stackedSpriteIds;
			}

			// Public data
			uint32_t				m_baseTileSpriteId;
			std::vector<uint32_t>	m_stackedSpriteIds;
		};

		typedef std::unordered_map<Key, uint32_t, Key::Hasher> Table;

						TileStackCache(
							Manifest*						aManifest);
						~TileStackCache();

		uint32_t		GetSpriteId(
							uint32_t						aBaseTileSpriteId,
							const std::vector<uint32_t>&	aStackedSpriteIds);

		// Data access
		const Table&	GetTable() const { return m_table; }

	private:

		Manifest*					m_manifest;
		Table						m_table;
	};

}