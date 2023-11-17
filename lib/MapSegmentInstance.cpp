#include "Pcheader.h"

#include <tpublic/Image.h>
#include <tpublic/Manifest.h>
#include <tpublic/MapSegmentInstance.h>

namespace tpublic
{

	void		
	MapSegmentInstance::Generate(
		const Manifest*				/*aManifest*/,
		const Data::MapSegment*		aMapSegment,
		std::mt19937&				/*aRandom*/)
	{
		m_mapSegment = aMapSegment;

		if(aMapSegment->m_tileMap)
		{
			// Already has a tile map
			m_tileMap = &aMapSegment->m_tileMap->m_tiles[0];			
			m_size = aMapSegment->m_tileMap->m_size;

			for (const Data::MapSegment::Object& object : aMapSegment->m_tileMap->m_objects)
			{
				switch(object.m_type)
				{
				case Data::MapSegment::OBJECT_TYPE_CONNECTOR:
					_GetOrCreateConnector(object.m_id)->m_positions.push_back(object.m_position);
					break;

				default:
					break;
				}
			}
		}
		else
		{
			// We need to generate a tile map
			assert(false); // FIXME
			// Don't forget to run tile map modifiers on it
		}
	}

	void		
	MapSegmentInstance::CreateDebugImage(
		const Manifest*				aManifest,
		Image&						aOut) const
	{
		aOut.Allocate((uint32_t)m_size.m_x, (uint32_t)m_size.m_y);

		// Tile map
		{
			Image::RGBA* out = aOut.GetData();
			const uint32_t* in = m_tileMap;
			for (int32_t y = 0; y < m_size.m_y; y++)
			{
				for (int32_t x = 0; x < m_size.m_x; x++)
				{
					if(*in != 0)
					{
						const Data::Sprite* sprite = aManifest->m_sprites.GetById(*in);

						if(sprite->m_info.m_flags & SpriteInfo::FLAG_TILE_WALKABLE)
							*out = { 64, 64, 64, 255 };
						else
							*out = { 128, 128, 128, 255 };
					}
					else
					{
						*out = { 0, 0, 0, 0 };
					}

					in++;
					out++;
				}
			}
		}

		// Connectors
		{
			Image::RGBA* out = aOut.GetData();
			for(const std::unique_ptr<Connector>& connector : m_connectors)
			{	
				for(const Vec2& position : connector->m_positions)
					out[position.m_x + position.m_y * m_size.m_x] = { 192, 192, 0, 255 };
			}
		}
	}

	const MapSegmentInstance::Connector* 
	MapSegmentInstance::GetConnector(
		uint32_t					aMapSegmentConnectorId) const
	{
		for (const std::unique_ptr<Connector>& connector : m_connectors)
		{
			if (connector->m_mapSegmentConnectorId == aMapSegmentConnectorId)
				return connector.get();
		}
		return NULL;
	}

	//--------------------------------------------------------------------------------

	MapSegmentInstance::Connector* 
	MapSegmentInstance::_GetOrCreateConnector(
		uint32_t					aMapSegmentConnectorId)
	{
		for (std::unique_ptr<Connector>& connector : m_connectors)
		{
			if(connector->m_mapSegmentConnectorId == aMapSegmentConnectorId)
				return connector.get();
		}

		Connector* t = new Connector();
		t->m_mapSegmentConnectorId = aMapSegmentConnectorId;
		m_connectors.push_back(std::unique_ptr<Connector>(t));
		return t;
	}

}