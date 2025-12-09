#include "Pcheader.h"

#include <tpublic/Data/Doodad.h>

#include <tpublic/AutoDoodads.h>
#include <tpublic/DoodadPlacement.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	AutoDoodads::AutoDoodads(
		const Manifest*										aManifest)
	{
		aManifest->GetContainer<Data::Doodad>()->ForEach([&](
			const Data::Doodad* aDoodad) -> bool
		{
			_InitDoodad(aDoodad);
			return true;
		});
	}

	AutoDoodads::~AutoDoodads()
	{

	}

	void	
	AutoDoodads::GenerateDoodads(
		uint32_t											aSeed,
		const uint32_t*										aTileMap,
		const uint32_t*										aCoverTileMap,
		const Vec2&											aSize,
		const Vec2&											aRegionPosition,
		const Vec2&											aRegionSize,		
		std::unordered_map<Vec2, uint32_t, Vec2::Hasher>&	aOutCoverageMap,
		Callback											aCallback) const
	{
		Vec2 regionMin = 
		{ 
			Base::Max(aRegionPosition.m_x, 0), 
			Base::Max(aRegionPosition.m_y, 0)
		};

		Vec2 regionMax = 
		{ 
			Base::Min(aRegionPosition.m_x + aRegionSize.m_x, aSize.m_x),
			Base::Min(aRegionPosition.m_y + aRegionSize.m_y, aSize.m_y)
		};

		for(int32_t y = regionMin.m_y; y < regionMax.m_y; y++)
		{
			for (int32_t x = regionMin.m_x; x < regionMax.m_x; x++)
			{
				Vec2 position = { x, y };

				if(!aOutCoverageMap.contains(position))
				{
					uint32_t tileSpriteId = aTileMap[x + y * aSize.m_x];
					TileTable::const_iterator i = m_tileTable.find(tileSpriteId);
					if (i != m_tileTable.cend())
					{
						const std::vector<const Data::Doodad*>* possibleDoodads = i->second.get();
						for (const Data::Doodad* possibleDoodad : *possibleDoodads)
						{
							bool canPlace = true;

							if(possibleDoodad->m_noCover && aCoverTileMap != NULL)
							{
								uint32_t coverTileSpriteId = aCoverTileMap[x + y * aSize.m_x];
								if(coverTileSpriteId != 0)
									canPlace = false;
							}

							if(canPlace)
								canPlace = possibleDoodad->m_spriteIds.size() > 0 && DoodadPlacement::Check(aTileMap, aSize, possibleDoodad, position, aOutCoverageMap);

							if(canPlace)
							{
								assert(possibleDoodad->m_autoProbability.has_value());
								uint32_t randomValue = (uint32_t)Hash::Splitmix_2_32(aSeed + possibleDoodad->m_id, position.GetHash32());
								uint32_t roll = (possibleDoodad->m_autoProbability->m_max * (randomValue >> 16)) / 0x10000;
								if(roll < possibleDoodad->m_autoProbability->m_min)
								{								
									DoodadPlacement::AddToCoverageMap(possibleDoodad, position, aOutCoverageMap);

									uint32_t spriteIndex = ((uint32_t)possibleDoodad->m_spriteIds.size() * (randomValue & 0xFFFF)) / 0x10000;

									aCallback(position, possibleDoodad->m_id, possibleDoodad->m_spriteIds[spriteIndex]);
								}
							}
						}
					}
				}
			}
		}
	}

	//-----------------------------------------------------------------------------------

	void	
	AutoDoodads::_InitDoodad(
		const Data::Doodad*									aDoodad)
	{
		if(!aDoodad->m_autoProbability.has_value())
			return;

		Vec2 center = { -aDoodad->m_mapOffset.m_x, -aDoodad->m_mapOffset.m_y };
		if(center.m_x < 0 || center.m_y < 0 || center.m_x >= aDoodad->m_mapSize.m_x || center.m_y >= aDoodad->m_mapSize.m_y)
			return;

		uint32_t mapPointDefinitionIndex = aDoodad->m_map[center.m_x + center.m_y * aDoodad->m_mapSize.m_x];
		assert(mapPointDefinitionIndex < aDoodad->m_mapPointDefinitions.size());
		const std::unique_ptr<Data::Doodad::MapPointDefinition>& mapPointDefinition = aDoodad->m_mapPointDefinitions[mapPointDefinitionIndex];

		for(uint32_t tileSpriteId : mapPointDefinition->m_mustBeOneOf)
			_GetOrCreateTileEntry(tileSpriteId)->push_back(aDoodad);
	}
	
	std::vector<const Data::Doodad*>* 
	AutoDoodads::_GetOrCreateTileEntry(
		uint32_t											aTileSpriteId)
	{
		TileTable::iterator i = m_tileTable.find(aTileSpriteId);
		if(i != m_tileTable.end())
			return i->second.get();

		std::vector<const Data::Doodad*>* t = new std::vector<const Data::Doodad*>();
		m_tileTable[aTileSpriteId] = std::unique_ptr<std::vector<const Data::Doodad*>>(t);
		return t;
	}

}