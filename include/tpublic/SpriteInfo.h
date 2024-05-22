#pragma once

#include "DataErrorHandling.h"
#include "Image.h"
#include "Vec2.h"

namespace tpublic
{

	class SpriteInfo
	{
	public:
		enum Flag : uint8_t
		{
			FLAG_TILE_WALKABLE				= 0x01,
			FLAG_TILE_BLOCK_LINE_OF_SIGHT	= 0x02,
			FLAG_TILE_FISHABLE				= 0x04,
			FLAG_TILE_TOP					= 0x08,
			FLAG_STANDALONE					= 0x10,
			FLAG_CENTERED					= 0x20,
			FLAG_DOUBLED					= 0x40
		};

		struct NamedAnchor
		{
			void
			ToStream(
				IWriter*	aStream) const 
			{
				aStream->WriteString(m_name);
				m_position.ToStream(aStream);
				m_size.ToStream(aStream);
			}

			bool
			FromStream(
				IReader*	aStream) 
			{
				if(!aStream->ReadString(m_name))
					return false;
				if (!m_position.FromStream(aStream))
					return false;
				if (!m_size.FromStream(aStream))
					return false;
				return true;
			}

			// Public data
			std::string				m_name;
			Vec2					m_position;
			Vec2					m_size;
		};

		static inline uint8_t 
		StringToFlag(			
			const char*		aString)
		{
			if (strcmp(aString, "tile_walkable") == 0)
				return FLAG_TILE_WALKABLE;
			if (strcmp(aString, "tile_block_line_of_sight") == 0)
				return FLAG_TILE_BLOCK_LINE_OF_SIGHT;
			if (strcmp(aString, "tile_fishable") == 0)
				return FLAG_TILE_FISHABLE;
			if (strcmp(aString, "tile_top") == 0)
				return FLAG_TILE_TOP;
			if (strcmp(aString, "standalone") == 0)
				return FLAG_STANDALONE;
			if (strcmp(aString, "centered") == 0)
				return FLAG_CENTERED;
			if (strcmp(aString, "doubled") == 0)
				return FLAG_DOUBLED;
			return 0;
		}

		void
		ToStream(
			IWriter*		aStream) const 
		{
			aStream->WritePOD(m_flags);
			aStream->WriteUInt(m_tileLayer);
			aStream->WriteUInt(m_deadSpriteId);
			aStream->WriteUInt(m_altGreyscaleSpriteId);
			aStream->WriteUInts(m_borders);
			m_origin.ToStream(aStream);
			aStream->WriteObjects(m_namedAnchors);
			aStream->WritePOD(m_averageColor);

			aStream->WriteUInt(m_borderTable.size());
			for(BorderTable::const_iterator i = m_borderTable.cbegin(); i != m_borderTable.cend(); i++)
			{
				aStream->WriteUInt(i->first);
				aStream->WriteUInts(*i->second);
			}
		}

		bool
		FromStream(
			IReader*		aStream) 
		{
			if (!aStream->ReadPOD(m_flags))
				return false;
			if (!aStream->ReadUInt(m_tileLayer))
				return false;
			if (!aStream->ReadUInt(m_deadSpriteId))
				return false;
			if (!aStream->ReadUInt(m_altGreyscaleSpriteId))
				return false;
			if (!aStream->ReadUInts(m_borders))
				return false;
			if(!m_origin.FromStream(aStream))
				return false;
			if(!aStream->ReadObjects(m_namedAnchors))
				return false;
			if(!aStream->ReadPOD(m_averageColor))
				return false;

			{
				size_t count;
				if(!aStream->ReadUInt(count))
					return false;
				for(size_t i = 0; i < count; i++)
				{
					uint32_t spriteId;
					if (!aStream->ReadUInt(spriteId))
						return false;
					std::shared_ptr<std::vector<uint32_t>> t = std::make_shared<std::vector<uint32_t>>();
					if(!aStream->ReadUInts(*t))
						return false;
					m_borderTable[spriteId] = std::move(t);
				}
			}

			return true;
		}

		const NamedAnchor&
		GetNamedAnchor(
			const char*		aName) const
		{
			for(const NamedAnchor& t : m_namedAnchors)
			{
				if(t.m_name == aName)
					return t;
			}
			TP_CHECK(false, "Named anchor '%s' not defined.", aName);
			static NamedAnchor dummy;
			return dummy;
		}

		std::vector<uint32_t>*
		GetOrCreateBorderArray(
			uint32_t		aSpriteId)
		{
			BorderTable::iterator i = m_borderTable.find(aSpriteId);
			if(i != m_borderTable.end())
				return i->second.get();
			std::vector<uint32_t>* t = new std::vector<uint32_t>();
			m_borderTable[aSpriteId] = std::shared_ptr<std::vector<uint32_t>>(t);
			return t;
		}

		const std::vector<uint32_t>&
		GetBorderArray(
			uint32_t		aSpriteId) const
		{
			BorderTable::const_iterator i = m_borderTable.find(aSpriteId);
			if(i != m_borderTable.cend())
				return *i->second;

			return m_borders;
		}

		// Public data
		uint8_t						m_flags = 0;
		uint32_t					m_tileLayer = 0;
		std::vector<uint32_t>		m_borders;
		Vec2						m_origin;
		std::vector<NamedAnchor>	m_namedAnchors;
		Image::RGBA					m_averageColor;
		uint32_t					m_deadSpriteId = 0;
		uint32_t					m_altGreyscaleSpriteId = 0;

		// This is a bit wonky, only using a shared_ptr here so SpriteInfo can be copied easily.
		typedef std::unordered_map<uint32_t, std::shared_ptr<std::vector<uint32_t>>> BorderTable;
		BorderTable					m_borderTable;
	};

}