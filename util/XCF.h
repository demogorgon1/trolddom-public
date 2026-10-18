#pragma once

#include <tpublic/Image.h>

namespace tpublic_util
{

	// Minimal reader for GIMP .xcf files. Only supports what map files use: 8-bit RGB(A) images with flat
	// layers. Anything else is reported as an error when trying to read the layer pixels.
	class XCF
	{
	public:
		struct Layer
		{
			std::string						m_name;
			uint32_t						m_width = 0;
			uint32_t						m_height = 0;
			uint32_t						m_type = 0;
			int32_t							m_offsetX = 0;
			int32_t							m_offsetY = 0;
			uint32_t						m_opacity = 255;
			uint32_t						m_mode = 0;
			bool							m_hasMode = false;
			bool							m_isGroup = false;
			bool							m_isInGroup = false;
			uint64_t						m_hierarchyOffset = 0;
			uint64_t						m_maskOffset = 0;
		};

						XCF(
							const char*				aPath);
						~XCF();

		const Layer*	GetLayerByName(
							const char*				aName) const;
		void			ReadLayerImage(
							const Layer*			aLayer,
							tpublic::Image&			aOut) const;

		// Public data
		std::string							m_path;
		uint32_t							m_version = 0;
		uint32_t							m_width = 0;
		uint32_t							m_height = 0;
		uint32_t							m_baseType = 0;
		uint32_t							m_precision = 0;
		uint8_t								m_compression = 0;
		std::vector<std::unique_ptr<Layer>>	m_layers;

	private:

		std::vector<uint8_t>				m_data;

		uint32_t		_ReadUInt32(
							size_t&					aOffset) const;
		uint64_t		_ReadPointer(
							size_t&					aOffset) const;
		std::string		_ReadString(
							size_t&					aOffset) const;
		void			_ReadTile(
							size_t					aOffset,
							size_t					aMaxSize,
							uint32_t				aBytesPerPixel,
							uint32_t				aNumPixels,
							std::vector<uint8_t>&	aOut) const;
	};

}
