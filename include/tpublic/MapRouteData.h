#pragma once

#include "DirectionField.h"
#include "IReader.h"
#include "IWriter.h"
#include "Vec2.h"

namespace tpublic
{

	class Manifest;

	class MapRouteData
	{
	public:		
		struct Route
		{
			void
			ToStream(
				IWriter*					aWriter) const
			{
				aWriter->WriteUInt(m_routeId);
				aWriter->WriteObjects(m_waypoints);
				m_directionField.ToStream(aWriter);
			}

			bool
			FromStream(
				IReader*					aReader)
			{
				if(!aReader->ReadUInt(m_routeId))
					return false;
				if(!aReader->ReadObjects(m_waypoints))
					return false;
				if(!m_directionField.FromStream(aReader))
					return false;
				return true;
			}

			// Public data
			uint32_t						m_routeId = 0;
			std::vector<Vec2>				m_waypoints;
			DirectionField					m_directionField;

			// Runtime (need prepare), not serialized
			typedef std::unordered_map<Vec2, size_t, Vec2::Hasher> WaypointToIndexTable;
			WaypointToIndexTable			m_waypointToIndexTable;

			// For building, not serialized
			typedef std::unordered_set<Vec2, Vec2::Hasher> Positions;
			Positions						m_positions;
		};

		void	ToStream(
					IWriter*				aWriter) const;
		bool	FromStream(
					IReader*				aReader);
		void	CopyFrom(
					const MapRouteData*		aOther);
		void	Prepare();
		Route*	GetOrCreateRoute(
					uint32_t				aRouteId);
		void	Build(
					const Manifest*			aManifest,
					const uint32_t*			aMapTiles,
					int32_t					aMapWidth,
					int32_t					aMapHeight);
		void	BuildRoute(
					const Manifest*			aManifest,
					const std::set<Vec2>&	aWalkable,
					int32_t					aMapWidth,
					int32_t					aMapHeight,
					Route*					aRoute);		
		bool	GetDirection(
					uint32_t				aRouteId,
					const Vec2&				aPosition,
					bool					aIsReversing,
					Vec2&					aOutDirection,
					bool&					aOutChangeDirection) const;

		// Public data
		std::vector<std::unique_ptr<Route>>	m_routes;

		typedef std::unordered_map<uint32_t, const Route*> RouteTable;
		RouteTable							m_routeTable;
	};

}