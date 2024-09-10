#include "Pcheader.h"

#include <tpublic/DataErrorHandling.h>
#include <tpublic/DirectionField.h>
#include <tpublic/DistanceField.h>
#include <tpublic/Image.h>
#include <tpublic/UniformDistribution.h>

namespace tpublic
{

	namespace
	{
		constexpr size_t 
		_PackedSize(
			const Vec2&			aSize)
		{
			size_t t = (size_t)(aSize.m_x * aSize.m_y);
			size_t packedSize = t / 4;
			if(t % 4)
				packedSize++;
			return packedSize;
		}
	}

	//--------------------------------------------------------------------------------------

	void		
	DirectionField::ToStream(
		IWriter*				aWriter) const
	{
		assert(_PackedSize(m_size) == m_packed.size());
		assert(m_packed.size() > 0);
		m_size.ToStream(aWriter);
		aWriter->Write(&m_packed[0], m_packed.size());
	}
	
	bool		
	DirectionField::FromStream(
		IReader*				aReader)
	{
		if(!m_size.FromStream(aReader))
			return false;
		m_packed.resize(_PackedSize(m_size));
		if(m_packed.size() == 0)
			return false;
		if(aReader->Read(&m_packed[0], m_packed.size()) != m_packed.size())
			return false;
		return true;
	}
	
	void		
	DirectionField::GenerateFromDistanceField(
		const DistanceField&	aDistanceField)
	{
		m_size.m_x = aDistanceField.m_width;
		m_size.m_y = aDistanceField.m_height;
		TP_CHECK(m_size.m_x * m_size.m_y > 0, "Invalid distance field.");
		m_packed.resize(_PackedSize(m_size));

		Vec2 neighbors[4];
		neighbors[DIRECTION_NORTH] = { 0, -1 };
		neighbors[DIRECTION_SOUTH] = { 0, 1 };
		neighbors[DIRECTION_WEST] = { -1, 0 };
		neighbors[DIRECTION_EAST] = { 1, 0 };

		for(int32_t y = 1; y < m_size.m_y - 1; y++)
		{
			for (int32_t x = 1; x < m_size.m_x - 1; x++)
			{
				Vec2 position = { x, y };

				uint32_t bestNeighborDistance = UINT32_MAX;
				std::vector<Direction> directions;

				for(uint8_t i = 0; i < 4; i++)
				{
					uint32_t neighborDistance = aDistanceField.Get(position + neighbors[i]);

					if(neighborDistance == bestNeighborDistance)
					{
						directions.push_back((Direction)i);
					}
					else if(neighborDistance < bestNeighborDistance)
					{						
						directions = { (Direction)i };
						bestNeighborDistance = neighborDistance;
					}
				}

				if(directions.size() > 0)
				{
					Direction direction = directions[0];

					if(directions.size() > 1)
					{
						UniformDistribution<size_t> distribution(0, directions.size() - 1);		
						direction = directions[distribution.Generate(position.GetHash32())];
					}

					size_t offset = (size_t)(x + y * m_size.m_x);

					size_t byteOffset = offset / 4;
					uint8_t bitOffset = (uint8_t)(offset % 4) * 2;

					assert(byteOffset < m_packed.size());
					m_packed[byteOffset] |= ((uint8_t)direction) << bitOffset;
				}
			}
		}
	}
	
	DirectionField::Direction
	DirectionField::GetDirection(
		const Vec2&				aPosition) const
	{
		assert(_PackedSize(m_size) == m_packed.size());
		assert(m_packed.size() > 0);

		if(aPosition.m_x < 0 || aPosition.m_y < 0 || aPosition.m_x >= m_size.m_x || aPosition.m_y >= m_size.m_y)
			return DIRECTION_NORTH;

		size_t offset = (size_t)(aPosition.m_x + aPosition.m_y * m_size.m_x);
		size_t byteOffset = offset / 4;
		uint8_t bitOffset = (uint8_t)(offset % 4) * 2;

		assert(byteOffset < m_packed.size());
		return (Direction)((m_packed[byteOffset] >> bitOffset) & 0x3);
	}

	void		
	DirectionField::SaveDebugPNG(		
		const std::set<Vec2>&	aWalkable,
		const std::set<Vec2>&	aOrigin,
		const char*				aPath) const
	{
		Image image;
		image.Allocate((uint32_t)m_size.m_x, (uint32_t)m_size.m_y);

		Image::RGBA* p = image.GetData();

		for(int32_t y = 0; y < m_size.m_y; y++)
		{
			for (int32_t x = 0; x < m_size.m_x; x++)
			{
				if(aWalkable.contains({ x, y }))
				{
					if(aOrigin.contains({ x, y }))
					{
						*p = { 255, 255, 255, 255 };
					}
					else
					{
						switch(GetDirection({ x, y }))
						{
						case DIRECTION_NORTH:	*p = { 255, 0, 0, 255 }; break;
						case DIRECTION_SOUTH:	*p = { 0, 255, 0, 255 }; break;
						case DIRECTION_WEST:	*p = { 0, 0, 255, 255 }; break;
						case DIRECTION_EAST:	*p = { 255, 255, 0, 255 }; break;

						default:
							assert(false);
						}
					}
				}

				p++;
			}
		}

		image.SavePNG(aPath);
	}

}