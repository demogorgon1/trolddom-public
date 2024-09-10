#pragma once

#include "Vec2.h"

namespace tpublic
{

	struct DistanceField;

	class DirectionField
	{
	public:
		enum Direction : uint8_t
		{
			DIRECTION_NORTH,
			DIRECTION_SOUTH,
			DIRECTION_WEST,
			DIRECTION_EAST
		};

		void		ToStream(
						IWriter*					aWriter) const;
		bool		FromStream(
						IReader*					aReader);
		void		GenerateFromDistanceField(
						const DistanceField&		aDistanceField);
		Direction	GetDirection(
						const Vec2&					aPosition) const;
		void		SaveDebugPNG(
						const std::set<Vec2>&		aWalkable,
						const std::set<Vec2>&		aOrigin,
						const char*					aPath) const;
	
		// Public data
		Vec2									m_size;				
		std::vector<uint8_t>					m_packed;
	};

}