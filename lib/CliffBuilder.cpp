#include "Pcheader.h"

#include <tpublic/Data/CliffStyle.h>

#include <tpublic/CliffBuilder.h>

namespace tpublic
{

	void		
	CliffBuilder::AddCliff(
		const Vec2&							aPosition,
		const Data::CliffStyle*				aCliffStyle,
		uint8_t								aElevation,
		bool								aRamp)
	{
		Cell t = { aCliffStyle, aElevation, aRamp };

		m_map[aPosition] = t;

		if(aElevation > 0)	
			m_highLevels.push_back({ aPosition, t });
	}

	void		
	CliffBuilder::Generate(
		std::function<void(const Tile&)>	aCallback) const
	{
		for(const std::pair<Vec2, Cell>& t : m_highLevels)
		{
			const Vec2& position = t.first;
			const Cell& cell = t.second;

			Data::CliffStyle::Cell surroundings[9];
			size_t index = 0;

			for(int32_t y = -1; y <= 1; y++)
			{
				for (int32_t x = -1; x <= 1; x++)
				{
					Map::const_iterator i = m_map.find(position + Vec2{ x, y });
					Data::CliffStyle::Cell c = Data::CliffStyle::CELL_LOW;
					if(i != m_map.cend())
					{
						if(i->second.m_elevation <= cell.m_elevation)
						{
							if(i->second.m_ramp)
								c = Data::CliffStyle::CELL_RAMP;
							else
								c = Data::CliffStyle::CELL_HIGH;
						}
					}

					surroundings[index++] = c;
				}
			}

			const Data::CliffStyle::Tile* picked = NULL;

			for(const std::unique_ptr<Data::CliffStyle::Tile>& tile : cell.m_cliffStyle->m_tiles)
			{
				bool match = true;

				for(size_t i = 0; i < 9 && match; i++)
				{
					Data::CliffStyle::Cell c = tile->m_cells[i];
					if(c != Data::CliffStyle::CELL_ANY && c != surroundings[i])
						match = false;
				}

				if(match)
				{
					picked = tile.get();
					break;
				}
			}

			if(picked != NULL && picked->m_sprites.size() > 0)
			{
				Tile output;
				output.m_position = position;

				if(picked->m_sprites.size() == 1)
				{
					output.m_spriteId = picked->m_sprites[0];
				}
				else
				{				
					// Randomize sprite based on position
					UniformDistribution<size_t> distribution(0, picked->m_sprites.size() - 1);
					output.m_spriteId = picked->m_sprites[distribution.Generate(position.GetHash32())];
				}
				
				aCallback(output);
			}
		}
	}


}