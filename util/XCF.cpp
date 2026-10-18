#include "Pcheader.h"

#include <lodepng/lodepng.h>

#include <ctype.h>

#include "XCF.h"

namespace tpublic_util
{

	namespace
	{

		enum Property : uint32_t
		{
			PROPERTY_END			= 0,
			PROPERTY_OPACITY		= 6,
			PROPERTY_MODE			= 7,
			PROPERTY_OFFSETS		= 15,
			PROPERTY_COMPRESSION	= 17,
			PROPERTY_GROUP_ITEM		= 29,
			PROPERTY_ITEM_PATH		= 30,
			PROPERTY_FLOAT_OPACITY	= 33
		};

		enum Compression : uint8_t
		{
			COMPRESSION_NONE,
			COMPRESSION_RLE,
			COMPRESSION_ZLIB
		};

		enum LayerType : uint32_t
		{
			LAYER_TYPE_RGB,
			LAYER_TYPE_RGBA
		};

		static const uint32_t BASE_TYPE_RGB = 0;
		static const uint32_t PRECISION_8_BIT_PERCEPTUAL = 150;
		static const uint32_t LAYER_MODE_NORMAL_LEGACY = 0;
		static const uint32_t LAYER_MODE_NORMAL = 28;
		static const uint32_t TILE_SIZE = 64;

	}

	//---------------------------------------------------------------------------------

	XCF::XCF(
		const char*		aPath)
		: m_path(aPath)
	{
		{
			FILE* f = fopen(aPath, "rb");
			TP_CHECK(f != NULL, "Failed to open file: %s", aPath);
			fseek(f, 0, SEEK_END);
			long size = ftell(f);
			fseek(f, 0, SEEK_SET);
			m_data.resize((size_t)size);
			size_t bytesRead = size > 0 ? fread(&m_data[0], 1, (size_t)size, f) : 0;
			fclose(f);
			TP_CHECK(bytesRead == (size_t)size, "Failed to read file: %s", aPath);
		}

		TP_CHECK(m_data.size() >= 14 && memcmp(&m_data[0], "gimp xcf ", 9) == 0, "Not an XCF file: %s", aPath);

		if(memcmp(&m_data[9], "file", 4) == 0)
		{
			m_version = 0;
		}
		else
		{
			TP_CHECK(m_data[9] == 'v' && isdigit(m_data[10]) && isdigit(m_data[11]) && isdigit(m_data[12]), "Invalid XCF version: %s", aPath);
			m_version = (uint32_t)((m_data[10] - '0') * 100 + (m_data[11] - '0') * 10 + (m_data[12] - '0'));
		}

		size_t offset = 14;
		m_width = _ReadUInt32(offset);
		m_height = _ReadUInt32(offset);
		m_baseType = _ReadUInt32(offset);

		if(m_version >= 4)
			m_precision = _ReadUInt32(offset);
		else
			m_precision = PRECISION_8_BIT_PERCEPTUAL;

		TP_CHECK(m_baseType == BASE_TYPE_RGB, "Only RGB images are supported: %s", aPath);
		TP_CHECK(m_version < 4 || (m_version >= 7 && m_precision == PRECISION_8_BIT_PERCEPTUAL), "Only 8-bit (perceptual gamma) images are supported: %s", aPath);

		// Image properties
		for(;;)
		{
			uint32_t type = _ReadUInt32(offset);
			uint32_t length = _ReadUInt32(offset);
			TP_CHECK(offset + length <= m_data.size(), "Unexpected end of file: %s", aPath);

			if(type == PROPERTY_END)
				break;

			if(type == PROPERTY_COMPRESSION)
			{
				TP_CHECK(length >= 1, "Invalid compression property: %s", aPath);
				m_compression = m_data[offset];
				TP_CHECK(m_compression <= COMPRESSION_ZLIB, "Unsupported compression: %s", aPath);
			}

			offset += length;
		}

		// Layers
		std::vector<uint64_t> layerOffsets;
		for(;;)
		{
			uint64_t layerOffset = _ReadPointer(offset);
			if(layerOffset == 0)
				break;
			layerOffsets.push_back(layerOffset);
		}

		for(uint64_t layerOffset : layerOffsets)
		{
			std::unique_ptr<Layer> layer = std::make_unique<Layer>();

			size_t p = (size_t)layerOffset;
			layer->m_width = _ReadUInt32(p);
			layer->m_height = _ReadUInt32(p);
			layer->m_type = _ReadUInt32(p);
			layer->m_name = _ReadString(p);

			for(;;)
			{
				uint32_t type = _ReadUInt32(p);
				uint32_t length = _ReadUInt32(p);
				TP_CHECK(p + length <= m_data.size(), "Unexpected end of file: %s", aPath);

				if(type == PROPERTY_END)
					break;

				size_t q = p;

				switch(type)
				{
				case PROPERTY_OPACITY:
					layer->m_opacity = _ReadUInt32(q);
					break;

				case PROPERTY_FLOAT_OPACITY:
					{
						uint32_t bits = _ReadUInt32(q);
						float opacity;
						memcpy(&opacity, &bits, sizeof(opacity));
						layer->m_opacity = (uint32_t)(opacity * 255.0f + 0.5f);
					}
					break;

				case PROPERTY_MODE:
					layer->m_mode = _ReadUInt32(q);
					layer->m_hasMode = true;
					break;

				case PROPERTY_OFFSETS:
					layer->m_offsetX = (int32_t)_ReadUInt32(q);
					layer->m_offsetY = (int32_t)_ReadUInt32(q);
					break;

				case PROPERTY_GROUP_ITEM:
					layer->m_isGroup = true;
					break;

				case PROPERTY_ITEM_PATH:
					layer->m_isInGroup = length > sizeof(uint32_t);
					break;

				default:
					break;
				}

				p += length;
			}

			layer->m_hierarchyOffset = _ReadPointer(p);
			layer->m_maskOffset = _ReadPointer(p);

			m_layers.push_back(std::move(layer));
		}
	}

	XCF::~XCF()
	{

	}

	const XCF::Layer*
	XCF::GetLayerByName(
		const char*			aName) const
	{
		for(const std::unique_ptr<Layer>& layer : m_layers)
		{
			if(layer->m_name == aName)
				return layer.get();
		}
		return NULL;
	}

	void
	XCF::ReadLayerImage(
		const Layer*		aLayer,
		tpublic::Image&		aOut) const
	{
		const char* name = aLayer->m_name.c_str();

		// Exported PNGs are the layer as it looks when shown on its own, so reject anything that would make that
		// different from the raw layer pixels.
		TP_CHECK(!aLayer->m_isGroup && !aLayer->m_isInGroup, "Layer groups are not supported: %s (%s)", m_path.c_str(), name);
		TP_CHECK(aLayer->m_type == LAYER_TYPE_RGB || aLayer->m_type == LAYER_TYPE_RGBA, "Only RGB(A) layers are supported: %s (%s)", m_path.c_str(), name);
		TP_CHECK(aLayer->m_maskOffset == 0, "Layer masks are not supported: %s (%s)", m_path.c_str(), name);
		TP_CHECK(aLayer->m_opacity == 255, "Layer opacity must be 100%%: %s (%s)", m_path.c_str(), name);
		TP_CHECK(!aLayer->m_hasMode || aLayer->m_mode == LAYER_MODE_NORMAL || aLayer->m_mode == LAYER_MODE_NORMAL_LEGACY, "Layer mode must be normal: %s (%s)", m_path.c_str(), name);

		uint32_t bytesPerPixel = aLayer->m_type == LAYER_TYPE_RGBA ? 4 : 3;

		// Hierarchy
		size_t p = (size_t)aLayer->m_hierarchyOffset;
		uint32_t width = _ReadUInt32(p);
		uint32_t height = _ReadUInt32(p);
		uint32_t hierarchyBytesPerPixel = _ReadUInt32(p);
		uint64_t levelOffset = _ReadPointer(p);

		TP_CHECK(width == aLayer->m_width && height == aLayer->m_height, "Layer size mismatch: %s (%s)", m_path.c_str(), name);
		TP_CHECK(hierarchyBytesPerPixel == bytesPerPixel, "Unexpected bytes per pixel: %s (%s)", m_path.c_str(), name);

		// Level
		p = (size_t)levelOffset;
		TP_CHECK(_ReadUInt32(p) == width && _ReadUInt32(p) == height, "Level size mismatch: %s (%s)", m_path.c_str(), name);

		uint32_t tilesX = (width + TILE_SIZE - 1) / TILE_SIZE;
		uint32_t tilesY = (height + TILE_SIZE - 1) / TILE_SIZE;

		std::vector<uint64_t> tileOffsets;
		for(;;)
		{
			uint64_t tileOffset = _ReadPointer(p);
			if(tileOffset == 0)
				break;
			tileOffsets.push_back(tileOffset);
		}

		TP_CHECK(tileOffsets.size() == (size_t)(tilesX * tilesY), "Unexpected number of tiles: %s (%s)", m_path.c_str(), name);

		aOut.Allocate(m_width, m_height);
		tpublic::Image::RGBA* out = aOut.GetData();

		std::vector<uint8_t> tile;

		for(uint32_t tileY = 0; tileY < tilesY; tileY++)
		{
			for(uint32_t tileX = 0; tileX < tilesX; tileX++)
			{
				size_t i = (size_t)(tileX + tileY * tilesX);
				size_t tileOffset = (size_t)tileOffsets[i];
				size_t tileEnd = i + 1 < tileOffsets.size() ? (size_t)tileOffsets[i + 1] : m_data.size();
				TP_CHECK(tileOffset < tileEnd && tileEnd <= m_data.size(), "Invalid tile offset: %s (%s)", m_path.c_str(), name);

				uint32_t x0 = tileX * TILE_SIZE;
				uint32_t y0 = tileY * TILE_SIZE;
				uint32_t tileWidth = (std::min)(TILE_SIZE, width - x0);
				uint32_t tileHeight = (std::min)(TILE_SIZE, height - y0);

				_ReadTile(tileOffset, tileEnd - tileOffset, bytesPerPixel, tileWidth * tileHeight, tile);

				for(uint32_t y = 0; y < tileHeight; y++)
				{
					int64_t canvasY = (int64_t)aLayer->m_offsetY + (int64_t)(y0 + y);
					if(canvasY < 0 || canvasY >= (int64_t)m_height)
						continue;

					for(uint32_t x = 0; x < tileWidth; x++)
					{
						int64_t canvasX = (int64_t)aLayer->m_offsetX + (int64_t)(x0 + x);
						if(canvasX < 0 || canvasX >= (int64_t)m_width)
							continue;

						const uint8_t* src = &tile[(size_t)(x + y * tileWidth) * bytesPerPixel];
						tpublic::Image::RGBA& dest = out[(size_t)canvasX + (size_t)canvasY * (size_t)m_width];
						dest.m_r = src[0];
						dest.m_g = src[1];
						dest.m_b = src[2];
						dest.m_a = bytesPerPixel == 4 ? src[3] : 255;
					}
				}
			}
		}
	}

	//---------------------------------------------------------------------------------

	uint32_t
	XCF::_ReadUInt32(
		size_t&				aOffset) const
	{
		TP_CHECK(aOffset + 4 <= m_data.size(), "Unexpected end of file: %s", m_path.c_str());
		const uint8_t* p = &m_data[aOffset];
		aOffset += 4;
		return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
	}

	uint64_t
	XCF::_ReadPointer(
		size_t&				aOffset) const
	{
		// Version 11 and later use 64-bit pointers
		if(m_version < 11)
			return (uint64_t)_ReadUInt32(aOffset);

		uint64_t high = (uint64_t)_ReadUInt32(aOffset);
		uint64_t low = (uint64_t)_ReadUInt32(aOffset);
		return (high << 32) | low;
	}

	std::string
	XCF::_ReadString(
		size_t&				aOffset) const
	{
		uint32_t length = _ReadUInt32(aOffset);
		if(length == 0)
			return std::string();

		TP_CHECK(aOffset + length <= m_data.size(), "Unexpected end of file: %s", m_path.c_str());

		// Length includes zero terminator
		std::string result((const char*)&m_data[aOffset], (size_t)length - 1);
		aOffset += length;
		return result;
	}

	void
	XCF::_ReadTile(
		size_t					aOffset,
		size_t					aMaxSize,
		uint32_t				aBytesPerPixel,
		uint32_t				aNumPixels,
		std::vector<uint8_t>&	aOut) const
	{
		size_t size = (size_t)aNumPixels * (size_t)aBytesPerPixel;
		aOut.resize(size);

		const uint8_t* in = &m_data[aOffset];
		const uint8_t* inEnd = in + aMaxSize;

		switch(m_compression)
		{
		case COMPRESSION_NONE:
			TP_CHECK(size <= aMaxSize, "Unexpected end of tile data: %s", m_path.c_str());
			memcpy(&aOut[0], in, size);
			break;

		case COMPRESSION_RLE:
			// Channels are stored separately, each one run-length encoded
			for(uint32_t channel = 0; channel < aBytesPerPixel; channel++)
			{
				uint32_t i = 0;
				while(i < aNumPixels)
				{
					TP_CHECK(in < inEnd, "Unexpected end of tile data: %s", m_path.c_str());
					uint32_t opcode = *(in++);

					if(opcode >= 128)
					{
						// Literal bytes
						uint32_t length = 256 - opcode;
						if(length == 128)
						{
							TP_CHECK(in + 2 <= inEnd, "Unexpected end of tile data: %s", m_path.c_str());
							length = ((uint32_t)in[0] << 8) | (uint32_t)in[1];
							in += 2;
						}

						TP_CHECK(i + length <= aNumPixels && in + length <= inEnd, "Invalid tile data: %s", m_path.c_str());
						for(uint32_t j = 0; j < length; j++)
							aOut[(size_t)(i++) * aBytesPerPixel + channel] = *(in++);
					}
					else
					{
						// Repeated byte
						uint32_t length = opcode + 1;
						if(length == 128)
						{
							TP_CHECK(in + 2 <= inEnd, "Unexpected end of tile data: %s", m_path.c_str());
							length = ((uint32_t)in[0] << 8) | (uint32_t)in[1];
							in += 2;
						}

						TP_CHECK(i + length <= aNumPixels && in < inEnd, "Invalid tile data: %s", m_path.c_str());
						uint8_t value = *(in++);
						for(uint32_t j = 0; j < length; j++)
							aOut[(size_t)(i++) * aBytesPerPixel + channel] = value;
					}
				}
			}
			break;

		case COMPRESSION_ZLIB:
			{
				// The size we're given can include data after the compressed stream, which would make the checksum
				// at the end of the input wrong. The decompressed size is checked instead.
				LodePNGDecompressSettings settings;
				lodepng_decompress_settings_init(&settings);
				settings.ignore_adler32 = 1;

				unsigned char* decompressed = NULL;
				size_t decompressedSize = 0;
				unsigned error = lodepng_zlib_decompress(&decompressed, &decompressedSize, in, aMaxSize, &settings);
				if(error == 0 && decompressedSize == size)
					memcpy(&aOut[0], decompressed, size);
				free(decompressed);

				TP_CHECK(error == 0, "Failed to decompress tile: %s (%s)", m_path.c_str(), lodepng_error_text(error));
				TP_CHECK(decompressedSize == size, "Unexpected tile size: %s", m_path.c_str());
			}
			break;

		default:
			TP_CHECK(false, "Unsupported compression: %s", m_path.c_str());
			break;
		}
	}

}
