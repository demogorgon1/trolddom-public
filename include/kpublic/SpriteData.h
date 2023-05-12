#pragma once

namespace tpublic
{

	class SpriteData
	{
	public:
		struct RGBA
		{
			uint8_t							m_r = 0;
			uint8_t							m_g = 0;
			uint8_t							m_b = 0;
			uint8_t							m_a = 0;
		};

		struct Sprite
		{
			uint32_t						m_sheetIndex = 0;
			const RGBA*						m_rgba = NULL;
			uint32_t						m_sheetX = 0;
			uint32_t						m_sheetY = 0;
			uint32_t						m_width = 0;
			uint32_t						m_height = 0;
		};

		struct Sheet
		{
			std::vector<Sprite>				m_sprites;
			std::uint32_t					m_width = 0;
			std::uint32_t					m_height = 0;
		};

					SpriteData();
					~SpriteData();

		bool		Load(
						const char*		aPath);

		// Public data
		std::vector<uint8_t>				m_data;
		std::vector<std::unique_ptr<Sheet>>	m_sheets;
	};

}