#include "Pcheader.h"

#include <tpublic/DistanceField.h>
#include <tpublic/Image.h>
#include <tpublic/UniformDistribution.h>
#include <tpublic/Vec2.h>

namespace tpublic
{

	DistanceField::DistanceField(
		int32_t					aWidth,
		int32_t					aHeight)
		: m_width(aWidth)
		, m_height(aHeight)
		, m_data(new uint32_t[aWidth * aHeight])
	{
		Clear();
	}

	DistanceField::~DistanceField()
	{
		delete[] m_data;
	}

	void
	DistanceField::Clear()
	{
		memset(m_data, 0xFF, sizeof(uint32_t) * m_width * m_height);
	}

	uint32_t
	DistanceField::Get(
		const Vec2&				aPosition) const
	{
		assert(aPosition.m_x >= 0 && aPosition.m_y >= 0 && aPosition.m_x < m_width && aPosition.m_y < m_height);
		return m_data[aPosition.m_x + aPosition.m_y * m_width];
	}

	uint32_t&
	DistanceField::GetReference(
		const Vec2&				aPosition) 
	{
		assert(aPosition.m_x >= 0 && aPosition.m_y >= 0 && aPosition.m_x < m_width && aPosition.m_y < m_height);
		return m_data[aPosition.m_x + aPosition.m_y * m_width];
	}

	void
	DistanceField::Set(
		const Vec2&				aPosition,
		uint32_t				aDistance)
	{
		assert(aPosition.m_x >= 0 && aPosition.m_y >= 0 && aPosition.m_x < m_width && aPosition.m_y < m_height);
		assert(aDistance != UINT32_MAX);
		m_data[aPosition.m_x + aPosition.m_y * m_width] = aDistance;
	}

	bool
	DistanceField::MakePath(
		std::mt19937&			aRandom,
		const Vec2&				aStartPosition,
		std::vector<Vec2>&		aOut) const
	{
		Vec2 position = aStartPosition;
		uint32_t distance = Get(position);

		while (distance > 0)
		{
			if (distance == UINT32_MAX)
				return false;

			Vec2 candidatePositions[4];
			uint32_t bestDistance = UINT32_MAX;
			uint32_t numCandidates = 0;

			static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
			for (size_t i = 0; i < 4; i++)
			{
				Vec2 p = { position.m_x + NEIGHBORS[i].m_x, position.m_y + NEIGHBORS[i].m_y };
				if (p.m_x >= 0 && p.m_y >= 0 && p.m_x < m_width && p.m_y < m_height)
				{
					uint32_t neighborDistance = Get(p);
					if (neighborDistance != UINT32_MAX && neighborDistance < distance)
					{
						if (neighborDistance < bestDistance)
						{
							candidatePositions[0] = p;
							bestDistance = neighborDistance;
							numCandidates = 1;
						}
						else if (neighborDistance == bestDistance)
						{
							assert(numCandidates < 4);
							candidatePositions[numCandidates++] = p;
						}
					}
				}
			}

			if (numCandidates == 0)
				return false;

			if (numCandidates == 1)
			{
				position = candidatePositions[0];
			}
			else
			{
				UniformDistribution<uint32_t> distribution(0, numCandidates - 1);
				position = candidatePositions[distribution(aRandom)];
			}

			distance = bestDistance;

			aOut.push_back(position);
		}

		return true;
	}

	void
	DistanceField::SaveDebugPNG(
		const char*			aPath) const
	{
		Image image;
		image.Allocate((uint32_t)m_width, (uint32_t)m_height);

		Image::RGBA* out = image.GetData();

		for (int32_t i = 0; i < m_width * m_height; i++)
		{
			uint32_t v = m_data[i];
			if (v > 0xFFFFFF)
				v = 0xFFFFFF;
			*out = { (uint8_t)(v & 0xFF), (uint8_t)((v & 0xFF00) >> 8), (uint8_t)((v & 0xFF0000) >> 16), 255 };
			out++;
		}

		image.SavePNG(aPath);
	}

	void		
	DistanceField::GenerateFromSinglePosition(
		const Vec2&										aPosition,
		const std::unordered_set<Vec2, Vec2::Hasher>&	aOpenSet,
		uint32_t										aMaxDistance)
	{
		typedef std::multimap<uint32_t, Vec2> Queue;
		Queue queue;
		queue.insert({ 0, aPosition });
		while (queue.size() > 0)
		{
			Queue::const_iterator front = queue.cbegin();
			uint32_t previousDistance = front->first;
			Vec2 position = front->second;
			queue.erase(front);

			uint32_t distance = previousDistance + 1;
			uint32_t& distanceMapReference = GetReference(position);

			if (distance < distanceMapReference && distance < aMaxDistance)
			{
				distanceMapReference = distance;

				static const Vec2 NEIGHBORS[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
				for (size_t j = 0; j < 4; j++)
				{
					Vec2 p = { position.m_x + NEIGHBORS[j].m_x, position.m_y + NEIGHBORS[j].m_y };
					if (aOpenSet.contains(p))
						queue.insert({ distance, p });
				}
			}
		}
	}

	void		
	DistanceField::Filter(
		std::function<uint32_t(uint32_t)>				aFunction)
	{
		for(int32_t i = 0, count = m_width * m_height; i < count; i++)
			m_data[i] = aFunction(m_data[i]);
	}

	void		
	DistanceField::CombineMin(
		const DistanceField*							aOther)
	{
		assert(aOther->m_width == m_width && aOther->m_height == m_height);

		for (int32_t i = 0, count = m_width * m_height; i < count; i++)
		{
			uint32_t otherValue = aOther->m_data[i];
			uint32_t thisValue = m_data[i];
			
			if(thisValue == UINT32_MAX || (otherValue != UINT32_MAX && otherValue < thisValue))
				m_data[i] = otherValue;
		}
	}

	uint32_t	
	DistanceField::GetMax() const
	{
		uint32_t maxValue = UINT32_MAX;
		for (int32_t i = 0, count = m_width * m_height; i < count; i++)
		{
			uint32_t value = m_data[i];
			if(value != UINT32_MAX && (maxValue == UINT32_MAX || value > maxValue))
				maxValue = value;
		}
		return maxValue;
	}
	
	void		
	DistanceField::GetPositionsWithValue(
		uint32_t										aValue,
		std::vector<Vec2>&								aOut) const
	{
		int32_t i = 0;
		for(int32_t y = 0; y < m_height; y++)
		{
			for(int32_t x = 0; x < m_width; x++)
			{
				if(m_data[i] == aValue)
					aOut.push_back({ x, y });
				i++;
			}
		}
	}

	void		
	DistanceField::GetPositionsMoreThanValue(
		uint32_t										aValue,
		std::vector<Vec2>&								aOut) const
	{
		int32_t i = 0;
		for(int32_t y = 0; y < m_height; y++)
		{
			for(int32_t x = 0; x < m_width; x++)
			{
				if(m_data[i] > aValue && m_data[i] != UINT32_MAX)
					aOut.push_back({ x, y });
				i++;
			}
		}
	}

}