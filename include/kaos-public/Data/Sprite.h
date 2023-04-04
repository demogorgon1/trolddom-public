#pragma once

#include "../DataBase.h"
#include "../SpriteInfo.h"

namespace kaos_public
{

	namespace Data
	{

		struct Sprite
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_SPRITE;

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const Parser::Node*		aNode) override
			{
				KP_VERIFY(false, aNode->m_debugInfo, "'sprite' must be defined in a sprite sheet.");
			}

			// Public data
			uint32_t	m_spriteSheetIndex = 0;
			uint32_t	m_offsetX = 0;
			uint32_t	m_offsetY = 0;
			uint32_t	m_width = 0;
			uint32_t	m_height = 0;
			SpriteInfo	m_info;
		};

	}

}