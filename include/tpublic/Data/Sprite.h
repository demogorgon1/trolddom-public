#pragma once

#include "../DataBase.h"
#include "../SpriteInfo.h"

namespace tpublic
{

	namespace Data
	{

		struct Sprite
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_SPRITE;
			static const bool TAGGED = true;

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{
				TP_VERIFY(false, aNode->m_debugInfo, "'sprite' must be defined in a sprite sheet.");
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInt(m_spriteSheetIndex);
				aStream->WriteUInt(m_offsetX);
				aStream->WriteUInt(m_offsetY);
				aStream->WriteUInt(m_width);
				aStream->WriteUInt(m_height);
				aStream->WriteUInt(m_index);
				m_info.ToStream(aStream);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInt(m_spriteSheetIndex))
					return false;
				if (!aStream->ReadUInt(m_offsetX))
					return false;
				if (!aStream->ReadUInt(m_offsetY))
					return false;
				if (!aStream->ReadUInt(m_width))
					return false;
				if (!aStream->ReadUInt(m_height))
					return false;
				if (!aStream->ReadUInt(m_index))
					return false;
				if (!m_info.FromStream(aStream))
					return false;
				return true;
			}

			// Public data
			uint32_t	m_spriteSheetIndex = 0;
			uint32_t	m_offsetX = 0;
			uint32_t	m_offsetY = 0;
			uint32_t	m_width = 0;
			uint32_t	m_height = 0;
			uint32_t	m_index = 0;
			SpriteInfo	m_info;
		};

	}

}