#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "PackedDistanceField.h"

namespace tpublic
{

	class MapPathData
	{
	public:
		static const size_t MAX_AREA_COUNT = 0xFFFF;

		static constexpr uint32_t 
		MakeAreaToAreaKey(
			uint32_t							aAId,
			uint32_t							aBId)
		{
			assert(aAId <= (uint32_t)MAX_AREA_COUNT);
			assert(aBId <= (uint32_t)MAX_AREA_COUNT);
			return (aAId << 16) | aBId;
		}

		static constexpr uint32_t
		GetAreaToAreaKeyA(
			uint32_t							aAreaToAreaKey)
		{
			return aAreaToAreaKey >> 16;
		}

		static constexpr uint32_t
		GetAreaToAreaKeyB(
			uint32_t							aAreaToAreaKey)
		{
			return aAreaToAreaKey & 0xFFFF;
		}

		struct Area
		{
			void
			ToStream(
				IWriter*						aWriter) const
			{
				aWriter->WriteUInt(m_id);
				m_origin.ToStream(aWriter);

				aWriter->WriteUInt(m_neighbors.size());
				for (Neighbors::const_iterator i = m_neighbors.cbegin(); i != m_neighbors.cend(); i++)
				{
					aWriter->WriteUInt(i->first);
					i->second->ToStream(aWriter);
				}
			}

			bool
			FromStream(
				IReader*						aReader)
			{
				if(!aReader->ReadUInt(m_id))
					return false;
				if(!m_origin.FromStream(aReader))
					return false;

				{
					size_t count;
					if(!aReader->ReadUInt(count))
						return false;

					for(size_t i = 0; i < count; i++)
					{
						uint32_t areaId;
						if(!aReader->ReadUInt(areaId))
							return false;

						if (m_neighbors.contains(areaId))
							return false;

						std::unique_ptr<PackedDistanceField> t = std::make_unique<PackedDistanceField>();
						if(!t->FromStream(aReader))
							return false;

						m_neighbors[areaId] = std::move(t);
					}
				}

				return true;
			}

			const PackedDistanceField*
			GetNeighborDistanceField(
				uint32_t						aId) const
			{
				Neighbors::const_iterator i = m_neighbors.find(aId);
				assert(i != m_neighbors.cend());
				return i->second.get();
			}

			// Public data
			uint32_t		m_id = 0;
			Vec2			m_origin;
			
			typedef std::unordered_map<uint32_t, std::unique_ptr<PackedDistanceField>> Neighbors;
			Neighbors		m_neighbors;
		};

		MapPathData()
		{

		}

		~MapPathData()
		{
			if(m_areaMap != NULL)
				delete [] m_areaMap;
		}

		void		CopyFrom(
						const MapPathData*		aOther);
		void		ToStream(
						IWriter*				aWriter) const;
		bool		FromStream(
						IReader*				aReader);
		void		CreateBlank(
						const Vec2&				aSize);
		uint32_t	GetAreaIdByPosition(
						const Vec2&				aPosition) const;
		const Area*	GetArea(
						uint32_t				aAreaId) const;
		bool		IsAreaBoundary(
						const Vec2&				aPositionA,
						const Vec2&				aPositionB) const;
		uint32_t	GetRouteNextAreaId(
						uint32_t				aCurrentAreaId,
						uint32_t				aDestinationAreaId) const;

		// Public data
		typedef std::unordered_map<uint32_t, std::unique_ptr<Area>> Areas;
		Areas					m_areas;
		
		typedef std::unordered_map<uint32_t, uint32_t> Routes;
		Routes					m_routes;

		Vec2					m_size;
		uint16_t*				m_areaMap = NULL;
	};

}