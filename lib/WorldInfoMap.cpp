#include "Pcheader.h"

#include <tpublic/Data/Sprite.h>

#include <tpublic/Manifest.h>
#include <tpublic/SpriteInfo.h>
#include <tpublic/WorldInfoMap.h>

namespace tpublic
{
	
	void 
	WorldInfoMap::AutoIndoor(
		const Manifest*		aManifest,
		int32_t				aMapWidth,
		int32_t				aMapHeight,
		const uint32_t*		aCoverMap,
		uint8_t*			aFlags)
	{		
		const uint32_t* coverMap = aCoverMap;
		uint8_t* flags = aFlags;

		for(int32_t i = 0, count = aMapWidth * aMapHeight; i < count; i++)
		{
			uint32_t tileSpriteId = *coverMap;
			if(tileSpriteId != 0)
			{
				const Data::Sprite* tileSprite = aManifest->GetById<Data::Sprite>(tileSpriteId);
				if(tileSprite->m_info.m_flags & SpriteInfo::FLAG_TILE_INDOOR)
					*flags |= FLAG_INDOOR;
			}

			coverMap++;
			flags++;
		}
	}

	void			
	WorldInfoMap::CopyFrom(
		const WorldInfoMap* aWorldInfoMap)
	{
		m_size = aWorldInfoMap->m_size;
		m_details = aWorldInfoMap->m_details;
		m_topLevelCells = aWorldInfoMap->m_topLevelCells;
		m_topLevelCellsSize = aWorldInfoMap->m_topLevelCellsSize;
		
		for(ZoneOutlineTable::const_iterator i = aWorldInfoMap->m_zoneOutlineTable.cbegin(); i != aWorldInfoMap->m_zoneOutlineTable.cend(); i++)
		{
			std::unique_ptr<ZoneOutline> t = std::make_unique<ZoneOutline>();
			t->m_positions = i->second->m_positions;
			m_zoneOutlineTable[i->first] = std::move(t);
		}
	}

	void			
	WorldInfoMap::Build(
		int32_t			aWidth,
		int32_t			aHeight,
		const uint32_t* aLevelMap,
		const uint32_t* aZoneMap,
		const uint32_t* aSubZoneMap,
		const uint8_t*	aFlagsMap)
	{		
		// Build maps
		{
			m_size = { aWidth, aHeight };
			m_topLevelCellsSize = { aWidth / TOP_LEVEL_CELL_SIZE, aHeight / TOP_LEVEL_CELL_SIZE };
			if (aWidth % TOP_LEVEL_CELL_SIZE)
				m_topLevelCellsSize.m_x++;
			if (aHeight % TOP_LEVEL_CELL_SIZE)
				m_topLevelCellsSize.m_y++;

			m_topLevelCells.resize(m_topLevelCellsSize.m_x * m_topLevelCellsSize.m_y);
			TopLevelCell* topLevelCell = &m_topLevelCells[0];

			for (int32_t y = 0; y < m_topLevelCellsSize.m_y; y++)
			{
				int32_t i0 = y * TOP_LEVEL_CELL_SIZE;
				int32_t i1 = Base::Min((y + 1) * TOP_LEVEL_CELL_SIZE, aHeight);

				for (int32_t x = 0; x < m_topLevelCellsSize.m_x; x++)
				{
					int32_t j0 = x * TOP_LEVEL_CELL_SIZE;
					int32_t j1 = Base::Min((x + 1) * TOP_LEVEL_CELL_SIZE, aWidth);

					std::optional<Entry> topLevelCellEntry;
					Details details;
					Entry* detailsEntry = details.m_entries;
					bool allTheSame = true;

					for (int32_t i = i0; i < i1; i++)
					{
						for (int32_t j = j0; j < j1; j++)
						{
							if(aLevelMap != NULL)
								detailsEntry->m_level = aLevelMap[j + i * aWidth];

							if(aZoneMap != NULL)
								detailsEntry->m_zoneId = aZoneMap[j + i * aWidth];								

							if (aSubZoneMap != NULL)
								detailsEntry->m_subZoneId = aSubZoneMap[j + i * aWidth];

							if (aFlagsMap != NULL)
								detailsEntry->m_flags = aFlagsMap[j + i * aWidth];

							if (topLevelCellEntry.has_value())
							{
								if (topLevelCellEntry.value() != *detailsEntry)
									allTheSame = false;
							}
							else
							{
								topLevelCellEntry = *detailsEntry;
							}

							detailsEntry++;
						}
					}

					if (!allTheSame)
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

		// Find zone outlines
		for(int32_t y = 0; y < aHeight; y++)
		{
			for(int32_t x = 0; x < aWidth; x++)
			{
				uint32_t zoneId = Get({ x, y }).m_zoneId;
				if(zoneId != 0)
				{
					if(Get({ x, y - 1 }).m_zoneId != zoneId || 
						Get({ x, y + 1 }).m_zoneId != zoneId ||
						Get({ x - 1, y }).m_zoneId != zoneId ||
						Get({ x + 1, y }).m_zoneId != zoneId)
					{
						ZoneOutlineTable::iterator i = m_zoneOutlineTable.find(zoneId);
						if(i != m_zoneOutlineTable.end())
						{
							i->second->m_positions.push_back({ x, y });
						}
						else
						{
							ZoneOutline* t = new ZoneOutline();
							t->m_positions.push_back({ x, y });
							m_zoneOutlineTable[zoneId] = std::unique_ptr<ZoneOutline>(t);
						}
					}
				}
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

	const WorldInfoMap::ZoneOutline* 
	WorldInfoMap::GetZoneOutline(
		uint32_t			aZoneId) const
	{
		ZoneOutlineTable::const_iterator i = m_zoneOutlineTable.find(aZoneId);
		if (i != m_zoneOutlineTable.cend())
			return i->second.get();
		return NULL;
	}

	void			
	WorldInfoMap::ToStream(
		IWriter*			aWriter) const
	{
		aWriter->WriteObjects(m_details);
		aWriter->WriteObjects(m_topLevelCells);
		m_topLevelCellsSize.ToStream(aWriter);
		m_size.ToStream(aWriter);

		aWriter->WriteUInt(m_zoneOutlineTable.size());
		for (ZoneOutlineTable::const_iterator i = m_zoneOutlineTable.cbegin(); i != m_zoneOutlineTable.cend(); i++)
		{
			aWriter->WriteUInt(i->first);
			i->second->ToStream(aWriter);
		}
	}
	
	bool			
	WorldInfoMap::FromStream(
		IReader*			aReader)
	{
		if (!aReader->ReadObjects(m_details, 128 * 1024))
			return false;
		if (!aReader->ReadObjects(m_topLevelCells, 128 * 1024))
			return false;
		if (!m_topLevelCellsSize.FromStream(aReader))
			return false;
		if (!m_size.FromStream(aReader))
			return false;

		{
			size_t count;
			if(!aReader->ReadUInt(count))
				return false;

			for(size_t i = 0; i < count; i++)
			{
				uint32_t zoneId;
				if(!aReader->ReadUInt(zoneId))
					return false;

				std::unique_ptr<ZoneOutline> t = std::make_unique<ZoneOutline>();
				if(!t->FromStream(aReader))
					return false;

				m_zoneOutlineTable[zoneId] = std::move(t);
			}
		}
		return true;
	}

}