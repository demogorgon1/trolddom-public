#pragma once

#include "Data/Sprite.h"

#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"

namespace tpublic
{

	class TileLayering
	{
	public:				
		void
		FromSource(
			const SourceNode*			aSource)
		{
			aSource->GetIdArray(DataType::ID_SPRITE, m_tiles);
		}

		void
		ToStream(
			IWriter*					aStream) const 
		{
			aStream->WriteUInts(m_tiles);
		}

		bool
		FromStream(
			IReader*					aStream) 
		{
			if(!aStream->ReadUInts(m_tiles))
				return false;
			return true;
		}

		uint32_t
		GetTileLayer(
			const Data::Sprite*			aSprite) const
		{
			if(aSprite->m_info.m_flags & SpriteInfo::FLAG_TILE_TOP)
				return (uint32_t)(m_tiles.size() + 1);

			for(size_t i = 0; i < m_tiles.size(); i++)
			{
				if(m_tiles[i] == aSprite->m_id)
					return (uint32_t)(i + 1);
			}

			return 0;
		}

		// Public data
		std::vector<uint32_t>			m_tiles;		
	};

}
