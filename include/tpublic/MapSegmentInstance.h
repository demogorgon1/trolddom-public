#pragma once

#include "Data/MapSegment.h"

#include "Vec2.h"

namespace tpublic
{

	class MapSegmentInstance
	{
	public:
		struct Connector
		{
			std::vector<Vec2>					m_positions;
		};

		void		Generate(						
						const Data::MapSegment*		aMapSegment,
						std::mt19937&				aRandom);		

		// Public data
		std::vector<uint32_t>					m_tileMap;
		std::vector<std::unique_ptr<Connector>>	m_connectors;
		
		typedef std::unordered_map<Vec2, const Connector*, Vec2::Hasher> ConnectorMap;
		ConnectorMap							m_connectorMap;
	};

}