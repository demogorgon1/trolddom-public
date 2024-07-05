#include "Pcheader.h"

#include <tpublic/MapPathData.h>

namespace tpublic
{

	void		
	MapPathData::CopyFrom(
		const MapPathData*		aOther)
	{
		assert(m_areaMap == NULL);

		m_routes = aOther->m_routes;
		m_size = aOther->m_size;
		size_t elems = (size_t)(m_size.m_x * m_size.m_y);
		m_areaMap = new uint16_t[elems];
		memcpy(m_areaMap, aOther->m_areaMap, elems * sizeof(uint16_t));

		for (Areas::const_iterator i = aOther->m_areas.cbegin(); i != aOther->m_areas.cend(); i++)
		{
			std::unique_ptr<Area> t = std::make_unique<Area>();
			t->m_id = i->second->m_id;
			t->m_origin = i->second->m_origin;

			for(Area::Neighbors::const_iterator j = i->second->m_neighbors.cbegin(); j != i->second->m_neighbors.cend(); j++)
			{
				std::unique_ptr<PackedDistanceField> k = std::make_unique<PackedDistanceField>();
				k->CopyFrom(j->second.get());
				t->m_neighbors[j->first] = std::move(k);
			}

			m_areas[i->first] = std::move(t);
		}
	}

	void
	MapPathData::ToStream(
		IWriter*				aWriter) const
	{
		assert(m_areaMap != NULL);

		m_size.ToStream(aWriter);
		aWriter->Write(m_areaMap, (size_t)(m_size.m_x * m_size.m_y) * sizeof(uint16_t));

		aWriter->WriteUInt(m_areas.size());
		for (Areas::const_iterator i = m_areas.cbegin(); i != m_areas.cend(); i++)
			i->second->ToStream(aWriter);

		aWriter->WriteUInt(m_routes.size());
		for(Routes::const_iterator i = m_routes.cbegin(); i != m_routes.cend(); i++)
		{
			aWriter->WriteUInt(i->first);
			aWriter->WriteUInt(i->second);
		}
	}

	bool
	MapPathData::FromStream(
		IReader*				aReader)
	{
		assert(m_areaMap == NULL);

		if (!m_size.FromStream(aReader))
			return false;

		{
			size_t elems = (size_t)(m_size.m_x * m_size.m_y);
			if (elems > 8192 * 8192) // some big number we should encounter
				return false;

			m_areaMap = new uint16_t[elems];
			if (aReader->Read(m_areaMap, elems * sizeof(uint16_t)) != elems * sizeof(uint16_t))
				return false;
		}

		{
			size_t count;
			if (!aReader->ReadUInt(count))
				return false;

			for (size_t i = 0; i < count; i++)
			{
				std::unique_ptr<Area> t = std::make_unique<Area>();
				if (!t->FromStream(aReader))
					return false;

				uint32_t areaId = t->m_id;

				if (m_areas.contains(areaId))
					return false;

				m_areas[areaId] = std::move(t);
			}
		}

		{
			size_t count;
			if (!aReader->ReadUInt(count))
				return false;

			for (size_t i = 0; i < count; i++)
			{
				uint32_t areaToAreaKey;
				if (!aReader->ReadUInt(areaToAreaKey))
					return false;

				uint32_t nextAreaId;
				if (!aReader->ReadUInt(nextAreaId))
					return false;

				m_routes[areaToAreaKey] = nextAreaId;
			}
		}

		return true;
	}

	void
	MapPathData::CreateBlank(
		const Vec2&				aSize)
	{
		assert(m_areaMap == NULL);
		m_size = aSize;
		size_t elems = (size_t)(m_size.m_x * m_size.m_y);
		m_areaMap = new uint16_t[elems];
		memset(m_areaMap, 0xFF, elems * sizeof(uint16_t));
	}

	uint32_t
	MapPathData::GetAreaIdByPosition(
		const Vec2&				aPosition) const
	{
		if (aPosition.m_x >= 0 && aPosition.m_y >= 0 && aPosition.m_x < m_size.m_x && aPosition.m_y < m_size.m_y)
		{
			int32_t i = aPosition.m_x + aPosition.m_y * m_size.m_x;
			uint16_t v = m_areaMap[i];

			if(v == UINT16_MAX)
				return UINT32_MAX;

			return (uint32_t)v;
		}

		return UINT32_MAX;
	}

	const MapPathData::Area*
	MapPathData::GetArea(
		uint32_t				aAreaId) const
	{
		Areas::const_iterator i = m_areas.find(aAreaId);
		if (i == m_areas.cend())
			return NULL;
		return i->second.get();
	}

	bool
	MapPathData::IsAreaBoundary(
		const Vec2&				aPositionA,
		const Vec2&				aPositionB) const
	{
		uint32_t areaA = GetAreaIdByPosition(aPositionA);
		uint32_t areaB = GetAreaIdByPosition(aPositionB);
		return areaA != areaB;
	}

	uint32_t	
	MapPathData::GetRouteNextAreaId(
		uint32_t				aCurrentAreaId,
		uint32_t				aDestinationAreaId) const
	{
		Routes::const_iterator i = m_routes.find(MakeAreaToAreaKey(aCurrentAreaId, aDestinationAreaId));
		if(i == m_routes.cend())
			return UINT32_MAX;

		return i->second;
	}

}