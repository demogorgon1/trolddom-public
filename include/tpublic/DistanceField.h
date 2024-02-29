#pragma once

#include "Vec2.h"

namespace tpublic
{

	struct DistanceField
	{
					DistanceField(
						int32_t											aWidth,
						int32_t											aHeight);
					~DistanceField();

		void		Clear();
		uint32_t	Get(
						const Vec2&										aPosition) const;
		uint32_t&	GetReference(
						const Vec2&										aPosition);
		void		Set(
						const Vec2&										aPosition,
						uint32_t										aDistance);
		bool		MakePath(
						std::mt19937&									aRandom,
						const Vec2&										aStartPosition,
						std::vector<Vec2>&								aOut) const;
		void		SaveDebugPNG(
						const char*										aPath) const;
		void		GenerateFromSinglePosition(
						const Vec2&										aPosition,
						const std::unordered_set<Vec2, Vec2::Hasher>&	aOpenSet,
						uint32_t										aMaxDistance);
		void		Filter(
						std::function<uint32_t(uint32_t)>				aFunction);
		void		CombineMin(
						const DistanceField*							aOther);
		uint32_t	GetMax() const;
		void		GetPositionsWithValue(
						uint32_t										aValue,
						std::vector<Vec2>&								aOut) const;

		// Public data
		int32_t		m_width = 0;
		int32_t		m_height = 0;
		uint32_t*	m_data = NULL;
	};

}