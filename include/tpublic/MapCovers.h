#pragma once

#include "Vec2.h"

namespace tpublic
{

	class IReader;
	class IWriter;

	class MapCovers
	{
	public:
		struct Entry
		{
			Entry()
			{

			}

			~Entry()
			{
				if(m_tileMap != NULL)
					delete [] m_tileMap;
			}

			void	
			ToStream(
				IWriter*				aWriter) const
			{
				m_position.ToStream(aWriter);
				m_size.ToStream(aWriter);
				aWriter->Write(m_tileMap, sizeof(uint32_t) * (size_t)(m_size.m_x * m_size.m_y));
			}
			
			bool	
			FromStream(
				IReader*				aReader)
			{
				if(!m_position.FromStream(aReader))
					return false;
				if(!m_size.FromStream(aReader))
					return false;
				assert(m_tileMap == NULL);
				m_tileMap = new uint32_t[m_size.m_x * m_size.m_y];
				size_t bytes = sizeof(uint32_t) * (size_t)(m_size.m_x * m_size.m_y);
				if(aReader->Read(m_tileMap, bytes) != bytes)
					return false;
				return true;
			}

			void
			Allocate(
				const Vec2&				aSize)
			{
				assert(m_tileMap == NULL);
				m_size = aSize;
				m_tileMap = new uint32_t[m_size.m_x * m_size.m_y];
				memset(m_tileMap, 0, sizeof(uint32_t) * (size_t)(m_size.m_x * m_size.m_y));
			}

			void
			CopyFrom(
				const Entry*			aEntry)
			{
				assert(m_tileMap == NULL);
				assert(aEntry->m_tileMap != NULL);
				m_position = aEntry->m_position;
				m_size = aEntry->m_size;
				m_tileMap = new uint32_t[m_size.m_x * m_size.m_y];
				memcpy(m_tileMap, aEntry->m_tileMap, sizeof(uint32_t) * (size_t)(m_size.m_x * m_size.m_y));
			}

			uint32_t
			GetTile(
				const Vec2&				aPosition) const
			{
				if(aPosition.m_x >= m_position.m_x && aPosition.m_y >= m_position.m_y && aPosition.m_x < m_position.m_x + m_size.m_x && aPosition.m_y < m_position.m_y + m_size.m_y)
				{
					Vec2 p = aPosition - m_position;
					return m_tileMap[p.m_x + p.m_y * m_size.m_x];
				}
				return 0;
			}

			// Public data
			Vec2								m_position;
			Vec2								m_size;
			uint32_t*							m_tileMap = NULL;
		};

		void	ToStream(
					IWriter*				aWriter) const;
		bool	FromStream(
					IReader*				aReader);
		void	Build(
					uint32_t				aWidth,
					uint32_t				aHeight,
					const uint32_t*			aTileMap);
		void	CopyFrom(
					const MapCovers*		aOther);
		void	ReconstructTileMap(
					const Vec2&				aSize,
					std::vector<uint32_t>&	aOutTileMap) const;
					
		// Public data
		std::vector<std::unique_ptr<Entry>>		m_entries;
	};

}