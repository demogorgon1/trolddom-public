#include "Pcheader.h"

#include <tpublic/Data/Sprite.h>

#include <tpublic/Image.h>
#include <tpublic/Manifest.h>
#include <tpublic/MapPathData.h>

#include "MapPathDataBuilder.h"

namespace tpublic
{

	MapPathDataBuilder::MapPathDataBuilder()
	{
	}
	
	MapPathDataBuilder::~MapPathDataBuilder()
	{

	}

	void
	MapPathDataBuilder::Build(
		const Manifest*		aManifest,
		const uint32_t*		aTileMap,
		uint32_t			aWidth,
		uint32_t			aHeight,
		size_t				aMaxAreaToAreaRouteLength)
	{
		m_waypoints = std::make_unique<Waypoints>((int32_t)aWidth, (int32_t)aHeight);

		BitMap bitMap((int32_t)aWidth, (int32_t)aHeight);
		std::mt19937 random;

		_ConstructBitMapAndInitWaypoints(aManifest, aTileMap, random, bitMap);

		for (const Vec2& waypoint : m_waypoints->m_points)
		{
			Area* area = m_waypoints->CreateArea();
			area->m_flood.push_back(waypoint);
			area->m_min = waypoint;
			area->m_max = waypoint + Vec2{ 1, 1 };
			m_waypoints->m_totalFlood++;
		}

		while(m_waypoints->m_totalFlood > 0)
		{
			for(Waypoints::Areas::iterator i = m_waypoints->m_areas.begin(); i != m_waypoints->m_areas.end(); i++)
			{
				Area* area = i->second.get();

				if(!area->m_flood.empty())
				{
					assert(m_waypoints->m_totalFlood >= (uint32_t)area->m_flood.size());
					m_waypoints->m_totalFlood -= (uint32_t)area->m_flood.size();

					std::unordered_set<Vec2, Vec2::Hasher> newFlood;

					for(const Vec2& position : area->m_flood)
					{
						if(!bitMap.Get(position))
						{
							uint32_t result = m_waypoints->SetMapIfUnassigned(position, area->m_id);

							if(result == 0)
							{
								newFlood.insert({ position.m_x + 1, position.m_y });
								newFlood.insert({ position.m_x - 1, position.m_y });
								newFlood.insert({ position.m_x, position.m_y + 1 });
								newFlood.insert({ position.m_x, position.m_y - 1 });

								area->m_positions.insert(position);

								if (position.m_x < area->m_min.m_x)
									area->m_min.m_x = position.m_x;
								if (position.m_y < area->m_min.m_y)
									area->m_min.m_y = position.m_y;
								if (position.m_x + 1 > area->m_max.m_x)
									area->m_max.m_x = position.m_x + 1;
								if (position.m_y + 1 > area->m_max.m_y)
									area->m_max.m_y = position.m_y + 1;
							}
							else if(result != UINT32_MAX && result != area->m_id)
							{
								// Alraedy assigned to some other area, so we have a neighbor
								area->GetOrCreateNeighbor(result)->m_border.insert(position);
							}
						}
					}

					area->m_flood.clear();

					if(!newFlood.empty())
					{
						for(const Vec2& position : newFlood)
						{
							area->m_flood.push_back(position);
							m_waypoints->m_totalFlood++;
						}
					}
				}
			}
		}

		for (Waypoints::Areas::iterator i = m_waypoints->m_areas.begin(); i != m_waypoints->m_areas.end(); i++)
		{
			Area* area = i->second.get();
			assert(area->m_flood.empty());

			area->m_size = area->m_max - area->m_min;
			assert(area->m_size.m_x > 0 && area->m_size.m_y > 0);

			// Create distance fields to neighbors
			for(Area::Neighbors::iterator j = area->m_neighbors.begin(); j != area->m_neighbors.end(); j++)
			{
				Neighbor* neighbor = j->second.get();
				assert(neighbor->m_id != area->m_id);

				_CreateAreaNeighborDistanceField(area, neighbor);
			}

			// Create routes to other areas
			_CreateAreaToAreaRoutes(area, AreaToAreaRoute(), aMaxAreaToAreaRouteLength);
		}
	}

	void	
	MapPathDataBuilder::Export(
		MapPathData*		aOut) const
	{
		TP_CHECK(m_waypoints->m_areas.size() <= MapPathData::MAX_AREA_COUNT, "Too many map path data areas.");

		aOut->CreateBlank({ m_waypoints->m_mapWidth, m_waypoints->m_mapHeight });
		
		for (Waypoints::Areas::const_iterator i = m_waypoints->m_areas.cbegin(); i != m_waypoints->m_areas.cend(); i++)
		{
			const Area* area = i->second.get();

			std::unique_ptr<MapPathData::Area> t = std::make_unique<MapPathData::Area>();

			t->m_id = area->m_id;
			t->m_origin = area->m_min;

			for(const Vec2& position : area->m_positions)
			{
				int32_t offset = position.m_x + position.m_y * m_waypoints->m_mapWidth;
				assert(area->m_id <= MapPathData::MAX_AREA_COUNT);
				aOut->m_areaMap[offset] = (uint16_t)area->m_id;
			}
			
			for (Area::Neighbors::const_iterator j = area->m_neighbors.cbegin(); j != area->m_neighbors.cend(); j++)
			{
				const Neighbor* neighbor = j->second.get(); 

				std::unique_ptr<PackedDistanceField> packedDistanceField = std::make_unique<PackedDistanceField>();
				packedDistanceField->Create(&neighbor->m_distanceField[0], area->m_size);

				t->m_neighbors[neighbor->m_id] = std::move(packedDistanceField);
			}

			aOut->m_areas[area->m_id] = std::move(t);
		}

		for(AreaToAreaTable::const_iterator i = m_areaToAreaTable.cbegin(); i != m_areaToAreaTable.cend(); i++)
		{
			uint32_t areaToAreaKey = i->first;
			const AreaToAreaRouteStart& route = i->second;
			aOut->m_routes[areaToAreaKey] = route.m_id;
		}
	}

	//---------------------------------------------------------------

	void	
	MapPathDataBuilder::_ConstructBitMapAndInitWaypoints(
		const Manifest*		aManifest,
		const uint32_t*		aTileMap,
		std::mt19937&		aRandom,
		BitMap&				aOutBitMap)
	{
		for(int32_t y = 0; y < aOutBitMap.m_height; y++)
		{
			for(int32_t x = 0; x < aOutBitMap.m_width; x++)
			{
				uint32_t tileSpriteId = aTileMap[x + y * aOutBitMap.m_width];
				const Data::Sprite* sprite = aManifest->m_sprites.GetById(tileSpriteId);
				bool walkable = sprite->m_info.m_flags & SpriteInfo::FLAG_TILE_WALKABLE;

				if(!walkable)
				{
					aOutBitMap.SetUnchecked(Vec2(x, y));
				}
				else
				{
					uint32_t roll = aRandom() & 0xFF;

					if(roll < 32)
					{
						bool valid = true;

						for(int32_t i = -3; i <= 3 && valid; i++)
						{
							for (int32_t j = -3; j <= 3 && valid; j++)
							{
								if(m_waypoints->m_points.contains(Vec2(x + i, y + j)))
									valid = false;
							}
						}

						if(valid)
							m_waypoints->m_points.insert(Vec2(x, y));
					}
				}
					
			}
		}
	}

	void	
	MapPathDataBuilder::_BitMapToImage(
		const BitMap&		aBitMap,
		Image&				aOut)
	{
		aOut.Allocate((uint32_t)aBitMap.m_width, (uint32_t)aBitMap.m_height);

		Image::RGBA* out = aOut.GetData();

		for (int32_t y = 0; y < aBitMap.m_height; y++)
		{
			for (int32_t x = 0; x < aBitMap.m_width; x++)
			{
				if(aBitMap.GetUnchecked(Vec2(x, y)))
					*out = { 128, 128, 128, 255 };
				else
					*out = { 32, 32, 32, 255 };

				out++;
			}
		}
	}

	void	
	MapPathDataBuilder::_CreateAreaNeighborDistanceField(
		Area*				aArea,
		Neighbor*			aNeighbor)
	{
		aNeighbor->m_distanceField.resize(aArea->m_size.m_x * aArea->m_size.m_y, UINT32_MAX);

		struct QueueItem
		{
			bool
			operator ==(
				const QueueItem& aOther) const
			{
				return m_distance == aOther.m_distance && m_position == aOther.m_position;
			}

			bool
			operator <(
				const QueueItem& aOther) const
			{
				if(m_distance < aOther.m_distance)
					return true;
				else if(m_distance == aOther.m_distance && m_position.m_x < aOther.m_position.m_x)
					return true;
				else if (m_distance == aOther.m_distance && m_position.m_x == aOther.m_position.m_x && m_position.m_y < aOther.m_position.m_y)
					return true;
				return false;
			}

			// Public data
			uint32_t	m_distance = 0;
			Vec2		m_position;
		};

		std::set<QueueItem> queue;

		for(const Vec2& position : aNeighbor->m_border)
		{
			Vec2 north = position + Vec2{ 0, -1 };
			Vec2 east = position + Vec2{ 1, 0 };
			Vec2 south = position + Vec2{ 0, 1 };
			Vec2 west = position + Vec2{ -1, 0 };

			if(aArea->m_positions.contains(north))
				queue.insert({ 0, north });
			if (aArea->m_positions.contains(east))
				queue.insert({ 0, east });
			if (aArea->m_positions.contains(south))
				queue.insert({ 0, south });
			if (aArea->m_positions.contains(west))
				queue.insert({ 0, west });
		}

		while(!queue.empty())
		{					
			QueueItem queueItem = *queue.begin();
			queue.erase(queueItem);

			Vec2 localPosition = queueItem.m_position - aArea->m_min;
			assert(localPosition.m_x >= 0 && localPosition.m_y >= 0 && localPosition.m_x < aArea->m_size.m_x && localPosition.m_y < aArea->m_size.m_y);

			uint32_t& distance = aNeighbor->m_distanceField[localPosition.m_x + localPosition.m_y * aArea->m_size.m_x];

			if(queueItem.m_distance < distance)
			{
				distance = queueItem.m_distance;

				static const Vec2 DIRS[4] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
				for(size_t i = 0; i < 4; i++)
				{
					Vec2 nextPosition = queueItem.m_position + DIRS[i];
					
					if(aArea->m_positions.contains(nextPosition))
						queue.insert({ queueItem.m_distance + 1, nextPosition });
				}
			}
		}
	}

	void	
	MapPathDataBuilder::_CreateAreaToAreaRoutes(
		Area*					aArea,
		const AreaToAreaRoute&	aRoute,
		size_t					aMaxLength)
	{
		// FIXME: this is far from optimal, could come up with a faster algorithm
		if (aRoute.m_length > 1)
			_AddAreaToAreaRoute(aRoute);

		if(aRoute.m_length == aMaxLength)
			return;

		AreaToAreaRoute extendedRoute = aRoute;
		extendedRoute.Add(aArea->m_id);

		for (Area::Neighbors::const_iterator i = aArea->m_neighbors.cbegin(); i != aArea->m_neighbors.cend(); i++)
		{
			const Neighbor* neighbor = i->second.get();

			if(!extendedRoute.HasArea(neighbor->m_id))
				_CreateAreaToAreaRoutes(m_waypoints->GetArea(neighbor->m_id), extendedRoute, aMaxLength);
		}
	}

	void	
	MapPathDataBuilder::_AddAreaToAreaRoute(
		const AreaToAreaRoute&	aRoute)
	{
		assert(aRoute.m_length > 1);

		uint32_t areaToAreaKey = MapPathData::MakeAreaToAreaKey(aRoute.GetStart(), aRoute.GetEnd());
		
		AreaToAreaTable::iterator i = m_areaToAreaTable.find(areaToAreaKey);
		if(i != m_areaToAreaTable.end())
		{
			AreaToAreaRouteStart& t = i->second;
			if((uint32_t)aRoute.m_length < t.m_length)
			{
				t.m_length = (uint32_t)aRoute.m_length;
				t.m_id = aRoute.GetFirstStep();
			}
		}
		else
		{
			AreaToAreaRouteStart t;
			t.m_length = (uint32_t)aRoute.m_length;
			t.m_id = aRoute.GetFirstStep();
			m_areaToAreaTable[areaToAreaKey] = t;
		}
	}

}