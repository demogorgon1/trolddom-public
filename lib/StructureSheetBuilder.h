#pragma once

#include <tpublic/Data/MapPalette.h>

#include <tpublic/DataErrorHandling.h>
#include <tpublic/Image.h>
#include <tpublic/SourceNode.h>

namespace tpublic
{

	class Manifest;
	class PersistentIdTable;

	// Structures (map segments) painted in image layers, cut out like sprites in a sprite sheet. Each structure becomes
	// a paletted map segment, so it can be used anywhere a map segment defined in text can.
	class StructureSheetBuilder
	{
	public:
				StructureSheetBuilder();
				~StructureSheetBuilder();

		void	AddStructureSheet(
					const SourceNode*		aSource);
		void	ExportManifestData(
					PersistentIdTable*		aPersistentIdTable,
					Manifest*				aManifest);

	private:

		struct AnchorSpawn
		{
			std::string							m_anchor;
			std::vector<uint32_t>				m_mapEntitySpawnIds;
			DataErrorHandling::DebugInfo		m_debugInfo;
		};

		struct Layer
		{
			std::vector<uint32_t>				m_palettes;
			bool								m_cover = false;
			std::string							m_source;
			Image								m_image;
			DataErrorHandling::DebugInfo		m_debugInfo;
		};

		struct Sheet;

		struct Structure
		{
			const Sheet*						m_sheet = NULL;
			std::string							m_name;
			std::string							m_string;
			uint32_t							m_underlyingTileSpriteId = 0;
			uint32_t							m_x = 0;
			uint32_t							m_y = 0;
			uint32_t							m_width = 0;
			uint32_t							m_height = 0;
			std::vector<AnchorSpawn>			m_anchorSpawns;
			DataErrorHandling::DebugInfo		m_debugInfo;
		};

		struct Sheet
		{
			std::vector<std::unique_ptr<Layer>>		m_layers;
			std::vector<AnchorSpawn>				m_anchorSpawns;
			std::vector<std::unique_ptr<Structure>>	m_structures;
			DataErrorHandling::DebugInfo			m_debugInfo;
		};

		typedef std::unordered_map<uint32_t, const Data::MapPalette::Entry*> CombinedPalette;

		std::vector<std::unique_ptr<Sheet>>		m_sheets;

		static std::unique_ptr<Layer>	_ParseLayer(
											const SourceNode*			aSource);
		static void						_ParseSize(
											const SourceNode*			aSource,
											uint32_t&					aOutWidth,
											uint32_t&					aOutHeight);
		static void						_AddAnchorSpawn(
											std::vector<AnchorSpawn>&	aAnchorSpawns,
											const SourceNode*			aSource);
		void							_ExportStructure(
											PersistentIdTable*			aPersistentIdTable,
											Manifest*					aManifest,
											const Structure*			aStructure,
											const std::vector<CombinedPalette>& aLayerPalettes,
											std::unordered_set<std::string>& aOutAnchorNames);
	};

}
