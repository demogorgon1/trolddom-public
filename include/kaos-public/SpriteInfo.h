#pragma once

namespace kaos_public
{

	class SpriteInfo
	{
	public:
		enum Flag : uint8_t
		{
			FLAG_TILE_WALKABLE = 0x01
		};

		static inline uint8_t 
		StringToFlag(			
			const char*		aString)
		{
			if(strcmp(aString, "tile_walkable") == 0)
				return FLAG_TILE_WALKABLE;

			return 0;
		}

		SpriteInfo()
			: m_flags(0)
		{

		}

		// Public data
		uint8_t		m_flags;
	};

}