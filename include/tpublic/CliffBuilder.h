#pragma once

#include "Vec2.h"

namespace tpublic
{
	
	namespace Data
	{
		struct CliffStyle;
	}

	class CliffBuilder
	{
	public:
		struct Tile
		{
			Vec2							m_position;
			uint32_t						m_spriteId = 0;
		};

		void		AddCliff(
						const Vec2&							aPosition,
						const Data::CliffStyle*				aCliffStyle,
						int8_t								aElevation,
						bool								aRamp);		
		void		Generate(
						std::function<void(const Tile&)>	aCallback) const;

	private:

		struct Cell
		{
			const Data::CliffStyle*			m_cliffStyle = NULL;
			int8_t							m_elevation = 0;
			bool							m_ramp = false;
		};

		typedef std::unordered_map<Vec2, Cell, Vec2::Hasher> Map;
		Map									m_map;

		std::vector<std::pair<Vec2, Cell>>	m_highLevels;
	};

}