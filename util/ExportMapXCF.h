#pragma once

namespace tpublic_util
{

	struct ExportMapXCFResult
	{
		uint32_t	m_numChanged = 0;	// PNG files that didn't match their layers (written unless in check mode)
		uint32_t	m_numErrors = 0;	// Layers that couldn't be exported because they're invalid
	};

	// Exports layers of map .xcf files (GIMP) to the PNG files used by maps. Paths can be .xcf files or
	// directories, which will be searched recursively. A layer is exported if a map in a .txt file next to the
	// .xcf file uses <xcf name>-<layer name>.png as the source of a layer or world map. Other references to PNG
	// files are ignored. PNG files are only written if the pixels have changed. In check mode nothing is written.
	// Map layers (but not world maps) must only have pixels with alpha 0 or 255, otherwise they're reported as
	// errors and not written.
	ExportMapXCFResult	ExportMapXCF(
							const std::vector<std::string>&		aPaths,
							bool								aCheckOnly);

}
