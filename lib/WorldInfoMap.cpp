#include "Pcheader.h"

#include <tpublic/WorldInfoMap.h>

namespace tpublic
{

	void			
	WorldInfoMap::CopyFrom(
		const WorldInfoMap* aWorldInfoMap)
	{
		m_size = aWorldInfoMap->m_size;
		m_details = aWorldInfoMap->m_details;
		m_topLevelCells = aWorldInfoMap->m_topLevelCells;
		m_topLevelCellsSize = aWorldInfoMap->m_topLevelCellsSize;
	}

	void			
	WorldInfoMap::Build(
		int32_t			aWidth,
		int32_t			aHeight,
		const uint32_t* aLevelMap,
		const uint32_t* aZoneMap)
	{		
		m_size = { aWidth, aHeight };
		m_topLevelCellsSize = { aWidth / TOP_LEVEL_CELL_SIZE, aHeight / TOP_LEVEL_CELL_SIZE };
		if(aWidth % TOP_LEVEL_CELL_SIZE)
			m_topLevelCellsSize.m_x++;
		if (aHeight % TOP_LEVEL_CELL_SIZE)
			m_topLevelCellsSize.m_y++;

		m_topLevelCells.resize(m_topLevelCellsSize.m_x * m_topLevelCellsSize.m_y);
		TopLevelCell* topLevelCell = &m_topLevelCells[0];

		for(int32_t y = 0; y < m_topLevelCellsSize.m_y; y++)
		{
			int32_t i0 = y * TOP_LEVEL_CELL_SIZE;
			int32_t i1 = Base::Min((y + 1) * TOP_LEVEL_CELL_SIZE, aHeight);

			for(int32_t x = 0; x < m_topLevelCellsSize.m_x; x++)
			{
				int32_t j0 = x * TOP_LEVEL_CELL_SIZE;
				int32_t j1 = Base::Min((x + 1) * TOP_LEVEL_CELL_SIZE, aWidth);

				std::optional<Entry> topLevelCellEntry;
				Details details;
				Entry* detailsEntry = details.m_entries;
				bool allTheSame = true;

				for(int32_t i = i0; i < i1; i++)
				{
					for (int32_t j = j0; j < j1; j++)
					{
						detailsEntry->m_level = aLevelMap[j + i * aWidth];
						detailsEntry->m_zoneId = aZoneMap[j + i * aWidth];

						if(topLevelCellEntry.has_value())
						{
							if(topLevelCellEntry.value() != *detailsEntry)
								allTheSame = false;
						}
						else
						{
							topLevelCellEntry = *detailsEntry;
						}

						detailsEntry++;
					}
				}

				if(!allTheSame)
				{
					topLevelCell->m_hasDetails = true;
					topLevelCell->m_detailsIndex = (uint32_t)m_details.size();
					m_details.push_back(details);
				}
				else
				{
					assert(topLevelCellEntry.has_value());
					topLevelCell->m_hasDetails = false;
					topLevelCell->m_entry = topLevelCellEntry.value();
				}

				topLevelCell++;
			}	
		}
	}
	
	const WorldInfoMap::Entry&
	WorldInfoMap::Get(
		const Vec2&			aPosition) const
	{
		if(aPosition.m_x < 0 || aPosition.m_y < 0 || aPosition.m_x >= m_size.m_x || aPosition.m_y >= m_size.m_y)
			return m_blankEntry;

		Vec2 topLevelCellPosition = { aPosition.m_x / TOP_LEVEL_CELL_SIZE, aPosition.m_y / TOP_LEVEL_CELL_SIZE };
		const TopLevelCell* topLevelCell = &m_topLevelCells[topLevelCellPosition.m_x + topLevelCellPosition.m_y * m_topLevelCellsSize.m_x];
		if(!topLevelCell->m_hasDetails)
			return topLevelCell->m_entry;

		Vec2 p = { aPosition.m_x % TOP_LEVEL_CELL_SIZE, aPosition.m_y % TOP_LEVEL_CELL_SIZE };

		assert((size_t)topLevelCell->m_detailsIndex < m_details.size());
		return m_details[topLevelCell->m_detailsIndex].m_entries[p.m_x + p.m_y * TOP_LEVEL_CELL_SIZE];
	}

	void			
	WorldInfoMap::ToStream(
		IWriter*			aWriter) const
	{
		aWriter->WriteObjects(m_details);
		aWriter->WriteObjects(m_topLevelCells);
		m_topLevelCellsSize.ToStream(aWriter);
		m_size.ToStream(aWriter);
	}
	
	bool			
	WorldInfoMap::FromStream(
		IReader*			aReader)
	{
		if (!aReader->ReadObjects(m_details))
			return false;
		if (!aReader->ReadObjects(m_topLevelCells, 128 * 1024))
			return false;
		if (!m_topLevelCellsSize.FromStream(aReader))
			return false;
		if (!m_size.FromStream(aReader))
			return false;
		return true;
	}

}