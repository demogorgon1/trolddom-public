#pragma once

#include <algorithm>

#include "Data/Zone.h"

#include "Image.h"
#include "IReader.h"
#include "IWriter.h"
#include "Manifest.h"

namespace tpublic
{

	// Ambient light color for each 4x4 block of tiles, from the zones' ambient light colors and blurred across zone
	// borders. Only built for maps where some zone has an ambient light color, other maps use the map's own color.
	class AmbientLightMap
	{
	public:
		static const int32_t CELL_SIZE = 4;
		static const int32_t BLUR_RADIUS = 1;		// In cells
		static const int32_t BLUR_PASSES = 2;
		static const int32_t MAX_CELLS = 512;		// Per side, maps are at most 2048 tiles

		static std::unique_ptr<AmbientLightMap>
		Build(
			const Manifest*							aManifest,
			int32_t									aWidth,
			int32_t									aHeight,
			const uint32_t*							aZoneMap,
			const uint32_t*							aSubZoneMap,
			const Image::RGBA&						aDefault)
		{
			if(aWidth <= 0 || aHeight <= 0 || (aZoneMap == NULL && aSubZoneMap == NULL))
				return NULL;

			// Zone colors, falling back to top zones
			std::unordered_map<uint32_t, std::optional<Image::RGBA>> zoneColors;
			auto getZoneColor = [&](
				uint32_t							aZoneId) -> std::optional<Image::RGBA>
			{
				std::unordered_map<uint32_t, std::optional<Image::RGBA>>::const_iterator i = zoneColors.find(aZoneId);
				if(i != zoneColors.cend())
					return i->second;

				std::optional<Image::RGBA> color;
				uint32_t zoneId = aZoneId;
				for(uint32_t depth = 0; depth < 16 && zoneId != 0 && !color.has_value(); depth++)
				{
					const Data::Zone* zone = aManifest->TryGetById<Data::Zone>(zoneId);
					if(zone == NULL)
						break;

					color = zone->m_ambientLight;
					zoneId = zone->m_topZoneId;
				}

				zoneColors[aZoneId] = color;
				return color;
			};

			std::unique_ptr<AmbientLightMap> t = std::make_unique<AmbientLightMap>();
			t->m_width = (aWidth + CELL_SIZE - 1) / CELL_SIZE;
			t->m_height = (aHeight + CELL_SIZE - 1) / CELL_SIZE;

			size_t cellCount = (size_t)(t->m_width * t->m_height);
			std::vector<float> rgb;
			rgb.resize(cellCount * 3, 0.0f);
			std::vector<uint32_t> tileCounts;
			tileCounts.resize(cellCount, 0);

			bool anyZoneColor = false;

			for(int32_t y = 0; y < aHeight; y++)
			{
				for(int32_t x = 0; x < aWidth; x++)
				{
					size_t i = (size_t)(x + y * aWidth);

					std::optional<Image::RGBA> color;
					if(aSubZoneMap != NULL && aSubZoneMap[i] != 0)
						color = getZoneColor(aSubZoneMap[i]);
					if(!color.has_value() && aZoneMap != NULL && aZoneMap[i] != 0)
						color = getZoneColor(aZoneMap[i]);

					if(color.has_value())
						anyZoneColor = true;
					else
						color = aDefault;

					size_t cell = (size_t)(x / CELL_SIZE + (y / CELL_SIZE) * t->m_width);
					rgb[cell * 3] += (float)color->m_r;
					rgb[cell * 3 + 1] += (float)color->m_g;
					rgb[cell * 3 + 2] += (float)color->m_b;
					tileCounts[cell]++;
				}
			}

			if(!anyZoneColor)
				return NULL;

			for(size_t i = 0; i < cellCount; i++)
			{
				float s = 1.0f / (float)tileCounts[i];
				rgb[i * 3] *= s;
				rgb[i * 3 + 1] *= s;
				rgb[i * 3 + 2] *= s;
			}

			// Box blur, horizontally and then vertically. Two passes look like a triangle filter.
			std::vector<float> temp;
			temp.resize(rgb.size());

			for(int32_t pass = 0; pass < BLUR_PASSES; pass++)
			{
				_Blur(rgb, temp, t->m_width, t->m_height, 1, 0);
				_Blur(temp, rgb, t->m_width, t->m_height, 0, 1);
			}

			t->m_cells.resize(cellCount * 3);
			for(size_t i = 0; i < rgb.size(); i++)
				t->m_cells[i] = (uint8_t)std::clamp<int32_t>((int32_t)(rgb[i] + 0.5f), 0, 255);

			return t;
		}

		void
		ToStream(
			IWriter*								aWriter) const
		{
			aWriter->WriteInt(m_width);
			aWriter->WriteInt(m_height);
			aWriter->Write(&m_cells[0], m_cells.size());
		}

		bool
		FromStream(
			IReader*								aReader)
		{
			if(!aReader->ReadInt(m_width))
				return false;
			if(!aReader->ReadInt(m_height))
				return false;
			if(m_width <= 0 || m_height <= 0 || m_width > MAX_CELLS || m_height > MAX_CELLS)
				return false;

			m_cells.resize((size_t)(m_width * m_height * 3));
			if(aReader->Read(&m_cells[0], m_cells.size()) != m_cells.size())
				return false;
			return true;
		}

		// Ambient light at a position in tiles (0.5 is the center of the first tile), blended between the four
		// nearest cells
		Image::RGBA
		Get(
			float									aX,
			float									aY) const
		{
			float cx = aX / (float)CELL_SIZE - 0.5f;
			float cy = aY / (float)CELL_SIZE - 0.5f;
			float fx = floorf(cx);
			float fy = floorf(cy);
			float tx = cx - fx;
			float ty = cy - fy;
			int32_t x0 = std::clamp<int32_t>((int32_t)fx, 0, m_width - 1);
			int32_t y0 = std::clamp<int32_t>((int32_t)fy, 0, m_height - 1);
			int32_t x1 = std::clamp<int32_t>((int32_t)fx + 1, 0, m_width - 1);
			int32_t y1 = std::clamp<int32_t>((int32_t)fy + 1, 0, m_height - 1);

			const uint8_t* c00 = &m_cells[(size_t)(x0 + y0 * m_width) * 3];
			const uint8_t* c10 = &m_cells[(size_t)(x1 + y0 * m_width) * 3];
			const uint8_t* c01 = &m_cells[(size_t)(x0 + y1 * m_width) * 3];
			const uint8_t* c11 = &m_cells[(size_t)(x1 + y1 * m_width) * 3];

			uint8_t out[3];
			for(int32_t i = 0; i < 3; i++)
			{
				float top = (float)c00[i] + ((float)c10[i] - (float)c00[i]) * tx;
				float bottom = (float)c01[i] + ((float)c11[i] - (float)c01[i]) * tx;
				out[i] = (uint8_t)std::clamp<int32_t>((int32_t)(top + (bottom - top) * ty + 0.5f), 0, 255);
			}

			return Image::RGBA(out[0], out[1], out[2], 255);
		}

	private:

		int32_t						m_width = 0;		// In cells
		int32_t						m_height = 0;
		std::vector<uint8_t>		m_cells;			// RGB per cell

		static void
		_Blur(
			const std::vector<float>&				aIn,
			std::vector<float>&						aOut,
			int32_t									aWidth,
			int32_t									aHeight,
			int32_t									aStepX,
			int32_t									aStepY)
		{
			for(int32_t y = 0; y < aHeight; y++)
			{
				for(int32_t x = 0; x < aWidth; x++)
				{
					float sum[3] = { 0.0f, 0.0f, 0.0f };

					// Clamped at the map edges
					for(int32_t j = -BLUR_RADIUS; j <= BLUR_RADIUS; j++)
					{
						int32_t sx = std::clamp<int32_t>(x + j * aStepX, 0, aWidth - 1);
						int32_t sy = std::clamp<int32_t>(y + j * aStepY, 0, aHeight - 1);
						size_t s = (size_t)(sx + sy * aWidth) * 3;
						sum[0] += aIn[s];
						sum[1] += aIn[s + 1];
						sum[2] += aIn[s + 2];
					}

					size_t o = (size_t)(x + y * aWidth) * 3;
					float scale = 1.0f / (float)(BLUR_RADIUS * 2 + 1);
					aOut[o] = sum[0] * scale;
					aOut[o + 1] = sum[1] * scale;
					aOut[o + 2] = sum[2] * scale;
				}
			}
		}
	};

}
