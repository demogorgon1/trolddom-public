#include "Pcheader.h"

#include <tpublic/Data/Doodad.h>

#include <tpublic/Vec2.h>

namespace tpublic
{

	namespace DoodadPlacement
	{

		void			
		AddToCoverageMap(
			const Data::Doodad*										aDoodad,
			const Vec2&												aDoodadPosition,
			std::unordered_map<Vec2, uint32_t, Vec2::Hasher>&		aCoverageMap)
		{
			size_t i = 0;
			for (int32_t y = 0; y < aDoodad->m_mapSize.m_y; y++)
			{
				for (int32_t x = 0; x < aDoodad->m_mapSize.m_x; x++)
				{
					const Data::Doodad::MapPointDefinition* doodadPoint = aDoodad->m_mapPointDefinitions[aDoodad->m_map[i++]].get();
					if(doodadPoint->m_occupy)
					{
						Vec2 p = { x + aDoodadPosition.m_x + aDoodad->m_mapOffset.m_x, y + aDoodadPosition.m_y + aDoodad->m_mapOffset.m_y };
						aCoverageMap[p] = 1;
					}
				}
			}
		}

		bool			
		Check(
			const uint32_t*											aMap,
			const Vec2&												aMapSize,
			const Data::Doodad*										aDoodad,
			const Vec2&												aDoodadPosition,
			const std::unordered_map<Vec2, uint32_t, Vec2::Hasher>& aCoverageMap)
		{
			if(aDoodadPosition.m_x + aDoodad->m_mapOffset.m_x < 0 || aDoodadPosition.m_y + aDoodad->m_mapOffset.m_y < 0)
				return false;
			if(aDoodadPosition.m_x + aDoodad->m_mapOffset.m_x + aDoodad->m_mapSize.m_x > aMapSize.m_x || aDoodadPosition.m_y + +aDoodad->m_mapOffset.m_y + aDoodad->m_mapSize.m_y > aMapSize.m_y)
				return false;

			size_t i = 0;

			for(int32_t y = 0; y < aDoodad->m_mapSize.m_y; y++)
			{
				for (int32_t x = 0; x < aDoodad->m_mapSize.m_x; x++)
				{
					const Data::Doodad::MapPointDefinition* doodadPoint = aDoodad->m_mapPointDefinitions[aDoodad->m_map[i++]].get();

					Vec2 p = { x + aDoodadPosition.m_x + aDoodad->m_mapOffset.m_x, y + aDoodadPosition.m_y + aDoodad->m_mapOffset.m_y };
					if(doodadPoint->m_occupy && aCoverageMap.contains(p))
						return false;

					uint32_t tileSpriteId = aMap[p.m_x + p.m_y * aMapSize.m_x];

					if(doodadPoint->m_mustBeOneOf.size() > 0)
					{
						bool matchTile = false;
						for(uint32_t t : doodadPoint->m_mustBeOneOf)
						{
							if(t == tileSpriteId)
							{
								matchTile = true;
								break;
							}
						}

						if(!matchTile)
							return false;
					}

					if (doodadPoint->m_mustNotBeOneOf.size() > 0)
					{
						for (uint32_t t : doodadPoint->m_mustNotBeOneOf)
						{
							if (t == tileSpriteId)
								return false;
						}
					}
				}
			}
			
			return true;
		}

	}

}