#include "Pcheader.h"

#include <tpublic/IReader.h>
#include <tpublic/IWriter.h>
#include <tpublic/MapCovers.h>

// DEBUG
#include <tpublic/Image.h>

namespace tpublic
{

	void	
	MapCovers::ToStream(
		IWriter*		aWriter) const
	{
		aWriter->WriteObjectPointers(m_entries);
	}
	
	bool	
	MapCovers::FromStream(
		IReader*		aReader)
	{
		if(!aReader->ReadObjectPointers(m_entries))
			return false;
		return true;
	}	

	void	
	MapCovers::Build(
		uint32_t		aWidth,
		uint32_t		aHeight,
		const uint32_t* aTileMap)
	{
		// We'll identify individual isolated covers within tilemap
		// FIXME: this is quite brutish, but it's offline, so meh
		std::unordered_set<Vec2, Vec2::Hasher> remainingPositions;

		{
			const uint32_t* p = aTileMap;
			for (int32_t y = 0; y < (int32_t)aHeight; y++)
			{
				for (int32_t x = 0; x < (int32_t)aWidth; x++)
				{
					if(*p != 0)
						remainingPositions.insert(Vec2{ x, y });

					p++;
				}
			}
		}

		struct TempEntry
		{
			Vec2									m_min;
			Vec2									m_max;
			std::unordered_set<Vec2, Vec2::Hasher>	m_positions;
		};

		std::vector<std::unique_ptr<TempEntry>> tempEntries;

		while(!remainingPositions.empty())
		{
			// Grab next position and flood fill from there
			Vec2 startPosition = *remainingPositions.cbegin();
			remainingPositions.erase(startPosition);

			std::unique_ptr<TempEntry> tempEntry = std::make_unique<TempEntry>();
			tempEntry->m_min = startPosition;
			tempEntry->m_max = startPosition;

			std::unordered_set<Vec2, Vec2::Hasher> queue;
			queue.insert(startPosition);

			while(!queue.empty())
			{
				Vec2 position = *queue.cbegin();
				queue.erase(position);

				tempEntry->m_positions.insert(position);
				if(position.m_x < tempEntry->m_min.m_x)
					tempEntry->m_min.m_x = position.m_x;
				if (position.m_y < tempEntry->m_min.m_y)
					tempEntry->m_min.m_y = position.m_y;
				if (position.m_x > tempEntry->m_max.m_x)
					tempEntry->m_max.m_x = position.m_x;
				if (position.m_y > tempEntry->m_max.m_y)
					tempEntry->m_max.m_y = position.m_y;

				static const Vec2 NEIGHBORS[4] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
				for(size_t i = 0; i < 4; i++)
				{
					Vec2 p = position + NEIGHBORS[i];
					if(p.m_x >= 0 && p.m_y >= 0 && p.m_x < (int32_t)aWidth && p.m_y < (int32_t)aHeight)
					{
						if(remainingPositions.contains(p) && !tempEntry->m_positions.contains(p))
						{
							queue.insert(p);
							remainingPositions.erase(p);
						}
					}
				}
			}			

			tempEntries.push_back(std::move(tempEntry));
		}

		for(const std::unique_ptr<TempEntry>& tempEntry : tempEntries)
		{
			std::unique_ptr<Entry> entry = std::make_unique<Entry>();
			entry->Allocate(tempEntry->m_max - tempEntry->m_min + Vec2{1, 1});
			entry->m_position = tempEntry->m_min;

			for(const Vec2& position : tempEntry->m_positions)
			{
				uint32_t tile = aTileMap[position.m_x + position.m_y * (int32_t)aWidth];
				Vec2 localPosition = position - tempEntry->m_min;
				entry->m_tileMap[localPosition.m_x + localPosition.m_y * entry->m_size.m_x] = tile;
			}

			m_entries.push_back(std::move(entry));
		}
	}

	void	
	MapCovers::CopyFrom(
		const MapCovers* aOther)
	{
		assert(m_entries.empty());
		for(const std::unique_ptr<Entry>& t : aOther->m_entries)
		{
			std::unique_ptr<Entry> entry = std::make_unique<Entry>();
			entry->CopyFrom(t.get());
			m_entries.push_back(std::move(entry));
		}
	}

	void	
	MapCovers::ReconstructTileMap(
		const Vec2&				aSize,
		std::vector<uint32_t>&	aOutTileMap) const
	{
		aOutTileMap.resize(aSize.m_x * aSize.m_y, 0);
		
		for (const std::unique_ptr<Entry>& t : m_entries)
		{
			assert(t->m_position.m_x + t->m_size.m_x <= aSize.m_x);
			assert(t->m_position.m_y + t->m_size.m_y <= aSize.m_y);

			uint32_t* out = &aOutTileMap[t->m_position.m_x + t->m_position.m_y * aSize.m_x];
			const uint32_t* in = t->m_tileMap;

			for(int32_t y = 0; y < t->m_size.m_y; y++)
			{
				for(int32_t x = 0; x < t->m_size.m_x; x++)
				{
					uint32_t tileSpriteId = in[x];
					if(tileSpriteId != 0)
						out[x] = tileSpriteId;
				}

				out += aSize.m_x;
				in += t->m_size.m_x;
			}
		}
	}

}