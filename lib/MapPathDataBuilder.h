#pragma once

#include <tpublic/Vec2.h>

namespace tpublic
{

	class Image;
	class Manifest;
	class MapPathData;

	class MapPathDataBuilder
	{
	public:		
				MapPathDataBuilder();
				~MapPathDataBuilder();

		void	Build(
					const Manifest*		aManifest,
					const uint32_t*		aTileMap,
					uint32_t			aWidth,
					uint32_t			aHeight,
					size_t				aMaxAreaToAreaRouteLength);
		void	Export(
					MapPathData*		aOut) const;

	private:

		struct BitMap
		{
			BitMap(
				int32_t					aWidth,
				int32_t					aHeight)
				: m_width(aWidth)
				, m_height(aHeight)
			{
				static_assert(sizeof(uint32_t) == 4);
				uint32_t elems = aWidth * aHeight;
				uint32_t count = (uint32_t)((elems / 32) + ((elems % 32) ? 1 : 0));
				m_bitMap = new uint32_t[count];
				memset(m_bitMap, 0, sizeof(uint32_t) * count);
			}

			~BitMap()
			{
				delete [] m_bitMap;
			}

			void
			Set(
				const Vec2&				aPosition)
			{
				if(aPosition.m_x >= 0 && aPosition.m_y >= 0 && aPosition.m_x < m_width && aPosition.m_y < m_height)
				{
					uint32_t i = (uint32_t)(aPosition.m_x + aPosition.m_y * m_width);
					m_bitMap[i / 32] |= (1 << (i % 32));
				}
			}

			void
			Clear(
				const Vec2&				aPosition)
			{
				if(aPosition.m_x >= 0 && aPosition.m_y >= 0 && aPosition.m_x < m_width && aPosition.m_y < m_height)
				{
					uint32_t i = (uint32_t)(aPosition.m_x + aPosition.m_y * m_width);
					m_bitMap[i / 32] &= ~(1 << (i % 32));
				}
			}

			bool
			Get(	
				const Vec2&				aPosition) const
			{
				if (aPosition.m_x >= 0 && aPosition.m_y >= 0 && aPosition.m_x < m_width && aPosition.m_y < m_height)
				{
					uint32_t i = (uint32_t)(aPosition.m_x + aPosition.m_y * m_width);
					return (m_bitMap[i / 32] & (1 << (i % 32))) != 0;
				}
				return true;
			}

			void
			SetUnchecked(
				const Vec2&				aPosition)
			{
				uint32_t i = (uint32_t)(aPosition.m_x + aPosition.m_y * m_width);
				m_bitMap[i / 32] |= (1 << (i % 32));
			}

			void
			ClearUnchecked(
				const Vec2&				aPosition)
			{
				uint32_t i = (uint32_t)(aPosition.m_x + aPosition.m_y * m_width);
				m_bitMap[i / 32] &= ~(1 << (i % 32));
			}

			bool
			GetUnchecked(
				const Vec2&				aPosition) const
			{
				uint32_t i = (uint32_t)(aPosition.m_x + aPosition.m_y * m_width);
				return (m_bitMap[i / 32] & (1 << (i % 32))) != 0;
			}

			// Public data
			uint32_t*								m_bitMap = NULL;
			int32_t									m_width = 0;
			int32_t									m_height = 0;
		};

		enum Direction : uint8_t
		{
			DIRECTION_NONE,
			DIRECTION_NORTH,
			DIRECTION_EAST,
			DIRECTION_SOUTH,
			DIRECTION_WEST
		};

		struct Neighbor
		{
			uint32_t								m_id = 0;
			std::unordered_set<Vec2, Vec2::Hasher>	m_border;
			std::vector<uint32_t>					m_distanceField;
		};

		struct Area
		{
			Neighbor*
			GetOrCreateNeighbor(
				uint32_t				aId)
			{
				Neighbors::iterator i = m_neighbors.find(aId);
				if(i != m_neighbors.end())
					return i->second.get();

				Neighbor* t = new Neighbor();
				t->m_id = aId;
				m_neighbors[aId] = std::unique_ptr<Neighbor>(t);
				return t;
			}

			// Public data
			uint32_t								m_id = 0;
			std::vector<Vec2>						m_flood;
			Vec2									m_min;
			Vec2									m_max;
			Vec2									m_size;

			typedef std::unordered_set<Vec2, Vec2::Hasher> Positions;
			Positions								m_positions;

			typedef std::unordered_map<uint32_t, std::unique_ptr<Neighbor>> Neighbors;
			Neighbors								m_neighbors;
		};

		struct Waypoints
		{	
			Waypoints(
				int32_t					aMapWidth,
				int32_t					aMapHeight)
				: m_mapWidth(aMapWidth)
				, m_mapHeight(aMapHeight)
			{
				m_areaMap.resize(aMapWidth * aMapHeight, 0);
			}

			Area*
			CreateArea()
			{
				Area* t = new Area();
				t->m_id = m_nextAreaId++;
				m_areas[t->m_id] = std::unique_ptr<Area>(t);
				return t;
			}

			Area*
			GetArea(
				uint32_t				aId)
			{
				Areas::iterator i = m_areas.find(aId);
				assert(i != m_areas.end());
				return i->second.get();
			}

			uint32_t
			SetMapIfUnassigned(
				const Vec2&				aPosition,
				uint32_t				aId)
			{
				if(aPosition.m_x >= 0 && aPosition.m_y >= 0 && aPosition.m_x < m_mapWidth && aPosition.m_y < m_mapHeight)
				{
					uint32_t& t = m_areaMap[aPosition.m_y * m_mapWidth + aPosition.m_x];
					if(t == 0)
					{
						t = aId;
						return 0; // Return zero if updated
					}
					else
					{
						return t; // Return existing value if not updated
					}
				}
				return UINT32_MAX; // Return UINT32_MAX if out of bounds
			}

			// Public data
			std::unordered_set<Vec2, Vec2::Hasher>	m_points;

			typedef std::unordered_map<uint32_t, std::unique_ptr<Area>> Areas;
			Areas									m_areas;
			std::vector<uint32_t>					m_areaMap;
			uint32_t								m_nextAreaId = 1;
			uint32_t								m_totalFlood = 0;
			int32_t									m_mapWidth = 0;
			int32_t									m_mapHeight = 0;
		};

		struct AreaToAreaRoute
		{
			static const size_t MAX_LENGTH = 14;
			
			bool
			HasArea(
				uint32_t				aAreaId) const
			{
				for(size_t i = 0; i < m_length; i++)
				{
					if(m_route[i] == aAreaId)
						return true;
				}
				return false;
			}

			void
			Add(
				uint32_t				aAreaId)
			{
				assert(m_length < MAX_LENGTH);
				m_route[m_length++] = aAreaId;
			}

			uint32_t
			GetStart() const
			{
				assert(m_length > 1);
				return m_route[0];
			}

			uint32_t
			GetEnd() const
			{
				assert(m_length > 1);
				return m_route[m_length - 1];
			}

			uint32_t
			GetFirstStep() const
			{
				assert(m_length > 1);
				return m_route[1];
			}

			// Public data
			uint32_t								m_route[MAX_LENGTH];
			size_t									m_length = 0;
		};

		struct AreaToAreaRouteStart
		{
			uint32_t								m_id = 0;
			uint32_t								m_length = 0;
		};

		typedef std::unordered_map<uint32_t, AreaToAreaRouteStart> AreaToAreaTable;
		AreaToAreaTable								m_areaToAreaTable;

		std::unique_ptr<Waypoints>					m_waypoints;
			
		void	_ConstructBitMapAndInitWaypoints(
					const Manifest*			aManifest,
					const uint32_t*			aTileMap,
					std::mt19937&			aRandom,
					BitMap&					aOutBitMap);
		void	_BitMapToImage(
					const BitMap&			aBitMap,
					Image&					aOut);
		void	_CreateAreaNeighborDistanceField(
					Area*					aArea,
					Neighbor*				aNeighbor);
		void	_CreateAreaToAreaRoutes(
					Area*					aArea,
					const AreaToAreaRoute&	aRoute,
					size_t					aMaxLength);
		void	_AddAreaToAreaRoute(
					const AreaToAreaRoute&	aRoute);
	};

}