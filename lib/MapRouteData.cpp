#include "Pcheader.h"

#include <tpublic/Data/Route.h>

#include <tpublic/DistanceField.h>
#include <tpublic/Manifest.h>
#include <tpublic/MapRouteData.h>

namespace tpublic
{

	void	
	MapRouteData::ToStream(
		IWriter*			aWriter) const
	{
		aWriter->WriteObjectPointers(m_routes);
	}
	
	bool	
	MapRouteData::FromStream(
		IReader*			aReader)
	{
		if(!aReader->ReadObjectPointers(m_routes))
			return false;
		return true;
	}
	
	void	
	MapRouteData::CopyFrom(
		const MapRouteData*	aOther)
	{
		for (const std::unique_ptr<Route>& t : aOther->m_routes)
		{
			std::unique_ptr<Route> copy = std::make_unique<Route>();
			copy->CopyFrom(t.get());
			m_routes.push_back(std::move(copy));
		}
	}

	void	
	MapRouteData::Prepare()
	{
		m_routeTable.clear();

		for (std::unique_ptr<Route>& t : m_routes)
		{
			m_routeTable[t->m_routeId] = t.get();

			for(std::unique_ptr<SubRoute>& subRoute : t->m_subRoutes)
			{
				subRoute->m_waypointToIndexTable.clear();

				for (size_t i = 0; i < subRoute->m_waypoints.size(); i++)
					subRoute->m_waypointToIndexTable[subRoute->m_waypoints[i]] = i;
			}
		}
	}

	MapRouteData::Route* 
	MapRouteData::GetOrCreateRoute(
		uint32_t			aRouteId)
	{
		for(std::unique_ptr<Route>& t : m_routes)
		{
			if(t->m_routeId == aRouteId)
				return t.get();
		}

		Route* route = new Route();
		route->m_routeId = aRouteId;
		m_routes.push_back(std::unique_ptr<Route>(route));
		return route;
	}

	void	
	MapRouteData::Build(
		const Manifest*		aManifest,
		const uint32_t*		aMapTiles,
		int32_t				aMapWidth,
		int32_t				aMapHeight,
		nwork::Queue*		aWorkQueue)
	{
		std::set<Vec2> walkable;

		const uint32_t* tile = aMapTiles;
		for(int32_t y = 0; y < aMapHeight; y++)		
		{
			for (int32_t x = 0; x < aMapWidth; x++)
			{
				const Data::Sprite* sprite = aManifest->GetById<Data::Sprite>(*tile);
				if(sprite->m_info.m_flags & SpriteInfo::FLAG_TILE_WALKABLE)
					walkable.insert({ x, y });

				tile++;
			}
		}

		aWorkQueue->ForEachVector<std::unique_ptr<Route>>(m_routes, [&](
			std::unique_ptr<Route>& aRoute)
		{
			BuildRoute(aManifest, walkable, aMapWidth, aMapHeight, aRoute.get());
		});
	}

	void	
	MapRouteData::BuildRoute(
		const Manifest*			aManifest,
		const std::set<Vec2>&	aWalkable,
		int32_t					aMapWidth,
		int32_t					aMapHeight,
		Route*					aRoute)
	{
		const Data::Route* routeData = aManifest->GetById<Data::Route>(aRoute->m_routeId);
		TP_UNUSED(routeData);

		static const Vec2 NEIGHBORS[4] = { { 1, 0 }, { -1, 0 }, { 0, 1}, { 0, -1 } };

		while(!aRoute->m_positions.empty())
		{
			std::unique_ptr<SubRoute> subRoute = std::make_unique<SubRoute>();

			Vec2 startPosition = *aRoute->m_positions.cbegin();

			for (const Vec2& position : aRoute->m_positions)
			{
				size_t neighborCount = 0;
				for (size_t i = 0; i < 4 && neighborCount < 2; i++)
				{
					Vec2 p = position + NEIGHBORS[i];
					if (aRoute->m_positions.contains(p))
						neighborCount++;
				}

				if (neighborCount == 1)
				{
					startPosition = position;
					break;
				}
			}

			Vec2 position = startPosition;

			while (aRoute->m_positions.size() > 0)
			{
				subRoute->m_waypoints.push_back(position);
				aRoute->m_positions.erase(position);

				std::optional<Vec2> nextPosition;

				for (size_t i = 0; i < 4; i++)
				{
					Vec2 p = position + NEIGHBORS[i];
					if (aRoute->m_positions.contains(p))
					{
						nextPosition = p;
						break;
					}
				}

				if (!nextPosition)
					break;

				position = nextPosition.value();
			}

			if(routeData->m_maxDirectionFieldDistance != 0)
			{
				DistanceField distanceField(aMapWidth, aMapHeight);
				distanceField.Generate(subRoute->m_waypoints, aWalkable, routeData->m_maxDirectionFieldDistance);

				subRoute->m_directionField = std::make_unique<DirectionField>();
				subRoute->m_directionField->GenerateFromDistanceField(distanceField);
			}

			if(subRoute->m_waypoints.size() >= 8)
			{
				int32_t startToEndDistanceSquared = subRoute->m_waypoints[0].DistanceSquared(subRoute->m_waypoints[subRoute->m_waypoints.size() - 1]);
				if(startToEndDistanceSquared == 1)
				{
					// Start and end are neighbors
					subRoute->m_isLoop = true;
				}
			}

			aRoute->m_subRoutes.push_back(std::move(subRoute));
		}
	}

	size_t	
	MapRouteData::GetSubRouteIndexByPosition(
		uint32_t				aRouteId,
		const Vec2&				aPosition) const
	{
		RouteTable::const_iterator i = m_routeTable.find(aRouteId);
		if (i == m_routeTable.cend())
			return SIZE_MAX;

		size_t index = 0;

		for(const std::unique_ptr<SubRoute>& subRoute : i->second->m_subRoutes)
		{
			SubRoute::WaypointToIndexTable::const_iterator j = subRoute->m_waypointToIndexTable.find(aPosition);
			if(j != subRoute->m_waypointToIndexTable.cend())
				return index;

			index++;
		}

		return SIZE_MAX;
	}

	bool	
	MapRouteData::GetDirection(
		uint32_t					aRouteId,
		size_t						aSubRouteIndex,
		const Vec2&					aPosition,
		bool						aIsReversing,
		Vec2&						aOutDirection,
		bool&						aOutChangeDirection,
		std::optional<uint32_t>&	aOutIndex) const
	{
		const Route* route = NULL;

		{
			RouteTable::const_iterator i = m_routeTable.find(aRouteId);
			if (i == m_routeTable.cend())
				return false;

			route = i->second;
		}

		const SubRoute* subRoute = NULL;
		if(aSubRouteIndex >= route->m_subRoutes.size())
			return false;

		subRoute = route->m_subRoutes[aSubRouteIndex].get();

		{
			SubRoute::WaypointToIndexTable::const_iterator i = subRoute->m_waypointToIndexTable.find(aPosition);
			if(i != subRoute->m_waypointToIndexTable.cend())
			{
				size_t index = i->second;
				size_t nextIndex = 0;

				if(index == subRoute->m_waypoints.size() - 1)
					aOutIndex = UINT32_MAX;
				else
					aOutIndex = (uint32_t)index;

				if(aIsReversing)
				{
					if(index == 0)
					{
						if(subRoute->m_isLoop)
						{
							nextIndex = subRoute->m_waypoints.size() - 1;
						}
						else
						{
							nextIndex = 1;
							aOutChangeDirection = true;
						}
					}
					else
					{
						nextIndex = index - 1;
					}
				}
				else
				{
					if(index == subRoute->m_waypoints.size() - 1)
					{
						if(subRoute->m_isLoop)
						{
							nextIndex = 0;
						}
						else
						{
							nextIndex = subRoute->m_waypoints.size() - 2;
							aOutChangeDirection = true;
						}
					}
					else
					{
						nextIndex = index + 1;
					}
				}

				if(nextIndex >= subRoute->m_waypoints.size())
					return false;

				aOutDirection = subRoute->m_waypoints[nextIndex] - aPosition;
			}
			else
			{
				if(!subRoute->m_directionField)
					return false;

				DirectionField::Direction direction = subRoute->m_directionField->GetDirection(aPosition);

				switch(direction)
				{
				case DirectionField::DIRECTION_NORTH:	aOutDirection = { 0, -1 }; break;
				case DirectionField::DIRECTION_SOUTH:	aOutDirection = { 0, 1 }; break;
				case DirectionField::DIRECTION_WEST:	aOutDirection = { -1, 0 }; break;
				case DirectionField::DIRECTION_EAST:	aOutDirection = { 1, 0 }; break;
				default:								assert(false);
				}
			}
		}

		return true;
	}

}