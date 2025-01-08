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
		struct SubRoute
		{
			void
			ToStream(
				IWriter*					aWriter) const
			{
				aWriter->WriteObjects(m_waypoints);
				aWriter->WriteOptionalObjectPointer(m_directionField);
				aWriter->WriteBool(m_isLoop);
			}

			bool
			FromStream(
				IReader*					aReader)
			{
				if(!aReader->ReadObjects(m_waypoints))
					return false;
				if(!aReader->ReadOptionalObjectPointer(m_directionField))
					return false;
				if(!aReader->ReadBool(m_isLoop))
					return false;
				return true;
			}

			void
			CopyFrom(
				const SubRoute&				aOther)
			{
				m_waypoints = aOther.m_waypoints;
				m_isLoop = aOther.m_isLoop;

				if(aOther.m_directionField)
				{
					m_directionField = std::make_unique<DirectionField>();
					*m_directionField = *aOther.m_directionField;
				}
			}

			// Public data
			std::vector<Vec2>						m_waypoints;
			std::unique_ptr<DirectionField>			m_directionField;
			bool									m_isLoop = false;

			// Runtime (need prepare), not serialized
			typedef std::unordered_map<Vec2, size_t, Vec2::Hasher> WaypointToIndexTable;
			WaypointToIndexTable					m_waypointToIndexTable;
		};

		struct Route
		{
			void
			ToStream(
				IWriter*					aWriter) const
			{
				aWriter->WriteUInt(m_routeId);
				aWriter->WriteObjectPointers(m_subRoutes);
			}

			bool
			FromStream(
				IReader*					aReader)
			{
				if(!aReader->ReadUInt(m_routeId))
					return false;
				if(!aReader->ReadObjectPointers(m_subRoutes))
					return false;
				return true;
			}

			void
			CopyFrom(	
				const Route*				aOther)
			{
				m_routeId = aOther->m_routeId;
				for(const std::unique_ptr<SubRoute>& subRoute : aOther->m_subRoutes)
				{
					std::unique_ptr<SubRoute> t = std::make_unique<SubRoute>();
					t->CopyFrom(*subRoute);
					m_subRoutes.push_back(std::move(t));
				}
				m_positions = aOther->m_positions;
			}

			// Public data
			uint32_t								m_routeId = 0;
			std::vector<std::unique_ptr<SubRoute>>	m_subRoutes;

			// For building, not serialized
			typedef std::unordered_set<Vec2, Vec2::Hasher> Positions;
			Positions								m_positions;
		};

		void	ToStream(
					IWriter*					aWriter) const;
		bool	FromStream(
					IReader*					aReader);
		void	CopyFrom(
					const MapRouteData*			aOther);
		void	Prepare();
		Route*	GetOrCreateRoute(
					uint32_t					aRouteId);
		void	Build(
					const Manifest*				aManifest,
					const uint32_t*				aMapTiles,
					int32_t						aMapWidth,
					int32_t						aMapHeight,
					nwork::Queue*				aWorkQueue);
		void	BuildRoute(
					const Manifest*				aManifest,
					const std::set<Vec2>&		aWalkable,
					int32_t						aMapWidth,
					int32_t						aMapHeight,
					Route*						aRoute);		
		size_t	GetSubRouteIndexByPosition(
					uint32_t					aRouteId,
					const Vec2&					aPosition) const;
		bool	GetDirection(
					uint32_t					aRouteId,
					size_t						aSubRouteIndex,
					const Vec2&					aPosition,
					bool						aIsReversing,
					Vec2&						aOutDirection,
					bool&						aOutChangeDirection,
					std::optional<uint32_t>&	aOutIndex) const;

		// Public data
		std::vector<std::unique_ptr<Route>>			m_routes;

		typedef std::unordered_map<uint32_t, const Route*> RouteTable;
		RouteTable									m_routeTable;
	};

}