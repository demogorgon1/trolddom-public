#pragma once

#include "DataErrorHandling.h"
#include "Image.h"
#include "Rarity.h"
#include "Vec2.h"

namespace tpublic
{

	class SpriteInfo
	{
	public:
		enum Flag : uint16_t
		{
			FLAG_TILE_WALKABLE				= 0x0001,
			FLAG_TILE_BLOCK_LINE_OF_SIGHT	= 0x0002,
			FLAG_TILE_FISHABLE				= 0x0004,
			FLAG_TILE_TOP					= 0x0008,
			FLAG_TILE_WATER					= 0x0010,
			FLAG_TILE_NO_WAVES				= 0x0020,
			FLAG_TILE_SNOW					= 0x0040,
			FLAG_STANDALONE					= 0x0080,
			FLAG_CENTERED					= 0x0100,
			FLAG_DOUBLED					= 0x0200
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

		struct IconMetaData
		{
			void
			ToStream(
				IWriter*	aStream) const 
			{
				aStream->WriteUInt(m_minLevel);
				aStream->WriteUInt(m_maxLevel);
				aStream->WritePOD(m_minRarity);
				aStream->WritePOD(m_maxRarity);
			}

			bool
			FromStream(
				IReader*	aStream) 
			{
				if (!aStream->ReadUInt(m_minLevel))
					return false;
				if (!aStream->ReadUInt(m_maxLevel))
					return false;
				if (!aStream->ReadPOD(m_minRarity))
					return false;
				if (!aStream->ReadPOD(m_maxRarity))
					return false;
				return true;
			}

			// Public data
			uint32_t				m_minLevel = 0;
			uint32_t				m_maxLevel = 0;
			Rarity::Id				m_minRarity = Rarity::INVALID_ID;
			Rarity::Id				m_maxRarity = Rarity::INVALID_ID;
		};

		static inline uint16_t 
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
			if (strcmp(aString, "tile_water") == 0)
				return FLAG_TILE_WATER;
			if (strcmp(aString, "tile_no_waves") == 0)
				return FLAG_TILE_NO_WAVES;
			if (strcmp(aString, "tile_snow") == 0)
				return FLAG_TILE_SNOW;
			if (strcmp(aString, "standalone") == 0)
				return FLAG_STANDALONE;
			if (strcmp(aString, "centered") == 0)
				return FLAG_CENTERED;
			if (strcmp(aString, "doubled") == 0)
				return FLAG_DOUBLED;
			return 0;
		}

		static inline uint16_t
		SourceToFlags(
			const SourceNode*	aSource)
		{
			uint16_t flags = 0;
			aSource->GetArray()->ForEachChild([&flags](
				const SourceNode* aChild)
			{
				uint16_t flag = StringToFlag(aChild->GetIdentifier());
				TP_VERIFY(flag != 0, aChild->m_debugInfo, "'%s' is not a valid sprite flag.", aChild->GetIdentifier());
				flags |= flag;
			});
			return flags;
		}

		void
		ToStream(
			IWriter*		aStream) const 
		{
			aStream->WritePOD(m_flags);
			aStream->WriteUInt(m_tileLayer);
			aStream->WriteUInt(m_deadSpriteId);
			aStream->WriteUInt(m_altGreyscaleSpriteId);
			aStream->WriteUInt(m_waterFloorSpriteId);
			aStream->WriteUInt(m_overviewMapOverrideSpriteId);
			aStream->WriteUInts(m_borders);
			aStream->WriteUInts(m_waterAnimationSpriteIds);
			aStream->WriteUInts(m_altTileSpriteIds);
			m_origin.ToStream(aStream);
			aStream->WriteObjects(m_namedAnchors);
			aStream->WritePOD(m_averageColor);
			aStream->WriteOptionalObject(m_iconMetaData);

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
			if (!aStream->ReadUInt(m_waterFloorSpriteId))
				return false;
			if (!aStream->ReadUInt(m_overviewMapOverrideSpriteId))
				return false;
			if (!aStream->ReadUInts(m_borders))
				return false;
			if (!aStream->ReadUInts(m_waterAnimationSpriteIds))
				return false;
			if (!aStream->ReadUInts(m_altTileSpriteIds))
				return false;
			if(!m_origin.FromStream(aStream))
				return false;
			if(!aStream->ReadObjects(m_namedAnchors))
				return false;
			if(!aStream->ReadPOD(m_averageColor))
				return false;
			if(!aStream->ReadOptionalObject(m_iconMetaData))
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

		bool
		FilterByIconMetaData(
			uint32_t		aLevel,
			Rarity::Id		aRarity) const
		{
			if(!m_iconMetaData)
				return false;

			if(m_iconMetaData->m_minLevel != 0 && aLevel < m_iconMetaData->m_minLevel)
				return false;

			if (m_iconMetaData->m_maxLevel != 0 && aLevel > m_iconMetaData->m_maxLevel)
				return false;

			if (m_iconMetaData->m_minRarity != Rarity::INVALID_ID && aRarity < m_iconMetaData->m_minRarity)
				return false;

			if (m_iconMetaData->m_maxRarity != Rarity::INVALID_ID && aRarity < m_iconMetaData->m_maxRarity)
				return false;

			return true;
		}

		// Public data
		uint16_t					m_flags = 0;
		uint32_t					m_tileLayer = 0;
		std::vector<uint32_t>		m_borders;
		Vec2						m_origin;
		std::vector<NamedAnchor>	m_namedAnchors;
		Image::RGBA					m_averageColor;
		uint32_t					m_deadSpriteId = 0;
		uint32_t					m_altGreyscaleSpriteId = 0;
		std::vector<uint32_t>		m_altTileSpriteIds;
		std::optional<IconMetaData>	m_iconMetaData;
		uint32_t					m_waterFloorSpriteId = 0;
		std::vector<uint32_t>		m_waterAnimationSpriteIds;
		uint32_t					m_overviewMapOverrideSpriteId = 0;

		// This is a bit wonky, only using a shared_ptr here so SpriteInfo can be copied easily.
		typedef std::unordered_map<uint32_t, std::shared_ptr<std::vector<uint32_t>>> BorderTable;
		BorderTable					m_borderTable;

		// Not serialized, client runtime only
		std::vector<uint32_t>		m_borderComposition;
		uint32_t					m_baseTileSpriteId = 0;
	};

}