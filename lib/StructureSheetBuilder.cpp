#include "Pcheader.h"

#include <tpublic/Data/MapSegment.h>

#include <tpublic/Manifest.h>
#include <tpublic/PersistentIdTable.h>

#include "StructureSheetBuilder.h"

namespace tpublic
{

	namespace
	{

		uint32_t
		_MakeColorKey(
			uint8_t					aR,
			uint8_t					aG,
			uint8_t					aB)
		{
			return ((uint32_t)aR << 16) | ((uint32_t)aG << 8) | (uint32_t)aB;
		}

	}

	StructureSheetBuilder::StructureSheetBuilder()
	{

	}

	StructureSheetBuilder::~StructureSheetBuilder()
	{

	}

	void
	StructureSheetBuilder::AddStructureSheet(
		const SourceNode*			aSource)
	{
		std::unique_ptr<Sheet> sheet = std::make_unique<Sheet>();
		sheet->m_debugInfo = aSource->m_debugInfo;

		uint32_t cursorX = 0;
		uint32_t cursorY = 0;
		uint32_t width = 0;
		uint32_t height = 0;

		aSource->GetObject()->ForEachChild([&](
			const SourceNode*		aNode)
		{
			if(aNode->m_name == "layers")
			{
				TP_VERIFY(sheet->m_layers.empty(), aNode->m_debugInfo, "Layers already defined.");

				aNode->GetArray()->ForEachChild([&](
					const SourceNode* aLayer)
				{
					sheet->m_layers.push_back(_ParseLayer(aLayer));
				});

				TP_VERIFY(sheet->m_layers.size() > 0, aNode->m_debugInfo, "No layers defined.");
			}
			else if(aNode->m_tag == "anchor")
			{
				_AddAnchorSpawn(sheet->m_anchorSpawns, aNode);
			}
			else if(aNode->m_name == "cursor")
			{
				aNode->GetObject()->ForEachChild([&](
					const SourceNode* aCursorNode)
				{
					if(aCursorNode->m_name == "position")
					{
						TP_VERIFY(aCursorNode->m_type == SourceNode::TYPE_ARRAY && aCursorNode->m_children.size() == 2, aCursorNode->m_debugInfo, "Invalid cursor position.");
						cursorX = aCursorNode->m_children[0]->GetUInt32();
						cursorY = aCursorNode->m_children[1]->GetUInt32();
					}
					else
					{
						TP_VERIFY(false, aCursorNode->m_debugInfo, "Invalid cursor item.");
					}
				});
			}
			else if(aNode->m_name == "size")
			{
				_ParseSize(aNode, width, height);
			}
			else if(aNode->m_name == "skip")
			{
				cursorX += width;
			}
			else if(aNode->m_tag == "structure")
			{
				TP_VERIFY(!aNode->m_name.empty(), aNode->m_debugInfo, "Structure must be named.");
				TP_VERIFY(sheet->m_layers.size() > 0, aNode->m_debugInfo, "Layers must be defined before structures.");

				std::unique_ptr<Structure> structure = std::make_unique<Structure>();
				structure->m_sheet = sheet.get();
				structure->m_name = aNode->m_name;
				structure->m_debugInfo = aNode->m_debugInfo;
				structure->m_x = cursorX;
				structure->m_y = cursorY;
				structure->m_width = width;
				structure->m_height = height;

				aNode->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_tag == "anchor")
						_AddAnchorSpawn(structure->m_anchorSpawns, aChild);
					else if(aChild->m_name == "size")
						_ParseSize(aChild, structure->m_width, structure->m_height);
					else if(aChild->m_name == "string")
						structure->m_string = aChild->GetString();
					else if(aChild->m_name == "underlying_tile")
						structure->m_underlyingTileSpriteId = aChild->GetId(DataType::ID_SPRITE);
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});

				TP_VERIFY(structure->m_width > 0 && structure->m_height > 0, aNode->m_debugInfo, "Structure must be of non-zero size.");

				for(const std::unique_ptr<Layer>& layer : sheet->m_layers)
				{
					TP_VERIFY(structure->m_x + structure->m_width <= layer->m_image.GetWidth() && structure->m_y + structure->m_height <= layer->m_image.GetHeight(), aNode->m_debugInfo,
						"Structure exceeds bounds of layer '%s'.", layer->m_source.c_str());
				}

				// Automatically advance horizontally
				cursorX += structure->m_width;

				sheet->m_structures.push_back(std::move(structure));
			}
			else
			{
				TP_VERIFY(false, aNode->m_debugInfo, "'%s' is not a valid item.", aNode->m_name.c_str());
			}
		});

		m_sheets.push_back(std::move(sheet));
	}

	void
	StructureSheetBuilder::ExportManifestData(
		PersistentIdTable*			aPersistentIdTable,
		Manifest*					aManifest)
	{
		for(const std::unique_ptr<Sheet>& sheet : m_sheets)
		{
			// Combine palettes for each layer
			std::vector<CombinedPalette> layerPalettes;

			for(const std::unique_ptr<Layer>& layer : sheet->m_layers)
			{
				CombinedPalette combinedPalette;

				for(uint32_t paletteId : layer->m_palettes)
				{
					const Data::MapPalette* palette = aManifest->GetContainer<Data::MapPalette>()->TryGetById(paletteId);
					TP_VERIFY(palette != NULL && palette->m_defined, layer->m_debugInfo, "Palette not defined (layer '%s').", layer->m_source.c_str());

					for(const Data::MapPalette::Entry& paletteEntry : palette->m_entries)
					{
						uint32_t key = _MakeColorKey(paletteEntry.m_color.m_r, paletteEntry.m_color.m_g, paletteEntry.m_color.m_b);
						TP_VERIFY(!combinedPalette.contains(key), layer->m_debugInfo, "Source color defined by multiple palettes used at the same time (layer '%s').", layer->m_source.c_str());
						combinedPalette[key] = &paletteEntry;
					}
				}

				layerPalettes.push_back(std::move(combinedPalette));
			}

			std::unordered_set<std::string> anchorNames;

			for(const std::unique_ptr<Structure>& structure : sheet->m_structures)
				_ExportStructure(aPersistentIdTable, aManifest, structure.get(), layerPalettes, anchorNames);

			for(const AnchorSpawn& anchorSpawn : sheet->m_anchorSpawns)
				TP_VERIFY(anchorNames.contains(anchorSpawn.m_anchor), anchorSpawn.m_debugInfo, "No structure in the sheet has an anchor called '%s'.", anchorSpawn.m_anchor.c_str());
		}
	}

	//---------------------------------------------------------------------------------

	std::unique_ptr<StructureSheetBuilder::Layer>
	StructureSheetBuilder::_ParseLayer(
		const SourceNode*			aSource)
	{
		std::unique_ptr<Layer> layer = std::make_unique<Layer>();
		layer->m_debugInfo = aSource->m_debugInfo;

		aSource->GetObject()->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "palettes")
			{
				aChild->GetIdArray(DataType::ID_MAP_PALETTE, layer->m_palettes);
			}
			else if(aChild->m_name == "source")
			{
				std::string path = aChild->m_realPath + "/" + aChild->m_value;
				layer->m_image.LoadPNG(path.c_str());
				layer->m_source = aChild->m_value;
			}
			else if(aChild->m_name == "cover")
			{
				layer->m_cover = aChild->GetBool();
			}
			else
			{
				TP_VERIFY(false, aChild->m_debugInfo, "Invalid layer item.");
			}
		});

		TP_VERIFY(layer->m_image.HasData(), aSource->m_debugInfo, "Missing 'source' item.");
		TP_VERIFY(layer->m_palettes.size() > 0, aSource->m_debugInfo, "No palettes defined for layer.");
		return layer;
	}

	void
	StructureSheetBuilder::_ParseSize(
		const SourceNode*			aSource,
		uint32_t&					aOutWidth,
		uint32_t&					aOutHeight)
	{
		aSource->GetObject()->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "width")
				aOutWidth = aChild->GetUInt32();
			else if (aChild->m_name == "height")
				aOutHeight = aChild->GetUInt32();
			else
				TP_VERIFY(false, aChild->m_debugInfo, "Invalid size component.");
		});
	}

	void
	StructureSheetBuilder::_AddAnchorSpawn(
		std::vector<AnchorSpawn>&	aAnchorSpawns,
		const SourceNode*			aSource)
	{
		for(const AnchorSpawn& anchorSpawn : aAnchorSpawns)
			TP_VERIFY(anchorSpawn.m_anchor != aSource->m_name, aSource->m_debugInfo, "Anchor '%s' already specified.", aSource->m_name.c_str());

		AnchorSpawn t;
		t.m_anchor = aSource->m_name;
		t.m_debugInfo = aSource->m_debugInfo;
		aSource->GetIdArray(DataType::ID_MAP_ENTITY_SPAWN, t.m_mapEntitySpawnIds);
		aAnchorSpawns.push_back(t);
	}

	void
	StructureSheetBuilder::_ExportStructure(
		PersistentIdTable*					aPersistentIdTable,
		Manifest*							aManifest,
		const Structure*					aStructure,
		const std::vector<CombinedPalette>&	aLayerPalettes,
		std::unordered_set<std::string>&	aOutAnchorNames)
	{
		const Sheet* sheet = aStructure->m_sheet;
		int32_t width = (int32_t)aStructure->m_width;
		int32_t height = (int32_t)aStructure->m_height;

		std::vector<uint32_t> tiles((size_t)(width * height), 0);
		std::vector<uint32_t> coverTiles((size_t)(width * height), 0);
		std::vector<Data::MapSegment::Object> objects;
		std::vector<Data::MapSegment::TileMap::DoodadObject> doodads;
		std::vector<Vec2> entrances;
		std::vector<Data::MapSegment::Anchor> anchors;

		// Read layers
		for(size_t layerIndex = 0; layerIndex < sheet->m_layers.size(); layerIndex++)
		{
			const Layer* layer = sheet->m_layers[layerIndex].get();
			const CombinedPalette& combinedPalette = aLayerPalettes[layerIndex];
			const Image::RGBA* pixels = layer->m_image.GetData();

			for(int32_t y = 0; y < height; y++)
			{
				for(int32_t x = 0; x < width; x++)
				{
					uint32_t imageX = aStructure->m_x + (uint32_t)x;
					uint32_t imageY = aStructure->m_y + (uint32_t)y;
					const Image::RGBA& pixel = pixels[imageX + imageY * layer->m_image.GetWidth()];
					if(pixel.m_a == 0)
						continue;

					TP_VERIFY(pixel.m_a == 255, aStructure->m_debugInfo, "Source alpha channel should be 0 or 255 (%u, %u) in layer '%s'.", imageX, imageY, layer->m_source.c_str());

					CombinedPalette::const_iterator i = combinedPalette.find(_MakeColorKey(pixel.m_r, pixel.m_g, pixel.m_b));
					TP_VERIFY(i != combinedPalette.cend(), aStructure->m_debugInfo, "Source color not defined in any palette: %u %u %u (%u, %u) in layer '%s'.",
						pixel.m_r, pixel.m_g, pixel.m_b, imageX, imageY, layer->m_source.c_str());

					const Data::MapPalette::Entry* entry = i->second;
					Vec2 position = { x, y };
					size_t index = (size_t)(x + y * width);

					if(layer->m_cover)
					{
						TP_VERIFY(entry->m_type == Data::MapPalette::ENTRY_TYPE_TILE, aStructure->m_debugInfo, "Cover layer can only have tiles (%u, %u) in layer '%s'.", imageX, imageY, layer->m_source.c_str());
						coverTiles[index] = entry->m_value;
						continue;
					}

					switch(entry->m_type)
					{
					case Data::MapPalette::ENTRY_TYPE_TILE:
						tiles[index] = entry->m_value;
						break;

					case Data::MapPalette::ENTRY_TYPE_ENTITY_SPAWN:
						objects.push_back({ position, Data::MapSegment::OBJECT_TYPE_ENTITY_SPAWN, entry->m_value });
						break;

					case Data::MapPalette::ENTRY_TYPE_PLAYER_SPAWN:
						objects.push_back({ position, Data::MapSegment::OBJECT_TYPE_PLAYER_SPAWN, entry->m_value });
						break;

					case Data::MapPalette::ENTRY_TYPE_PORTAL:
						objects.push_back({ position, Data::MapSegment::OBJECT_TYPE_PORTAL, entry->m_value });
						break;

					case Data::MapPalette::ENTRY_TYPE_DOODAD:
						doodads.push_back({ position, entry->m_value });
						break;

					case Data::MapPalette::ENTRY_TYPE_STRUCTURE_ENTRANCE:
						entrances.push_back(position);
						break;

					case Data::MapPalette::ENTRY_TYPE_STRUCTURE_ANCHOR:
						{
							Data::MapSegment::Anchor anchor;
							anchor.m_name = entry->m_string;
							anchor.m_position = position;
							anchors.push_back(anchor);
						}
						break;

					default:
						TP_VERIFY(false, aStructure->m_debugInfo, "Only tiles, entity spawns, player spawns, portals, doodads, structure entrances and structure anchors can be used in structures (%u, %u) in layer '%s'.",
							imageX, imageY, layer->m_source.c_str());
						break;
					}
				}
			}
		}

		// Trim to what's actually there. Entrances and anchors can be outside.
		Vec2 min = { INT32_MAX, INT32_MAX };
		Vec2 max = { INT32_MIN, INT32_MIN };

		auto include = [&](
			const Vec2&		aPosition)
		{
			min = { Base::Min(min.m_x, aPosition.m_x), Base::Min(min.m_y, aPosition.m_y) };
			max = { Base::Max(max.m_x, aPosition.m_x), Base::Max(max.m_y, aPosition.m_y) };
		};

		for(int32_t y = 0; y < height; y++)
		{
			for(int32_t x = 0; x < width; x++)
			{
				size_t index = (size_t)(x + y * width);
				if(tiles[index] != 0 || coverTiles[index] != 0)
					include({ x, y });
			}
		}

		for(const Data::MapSegment::Object& object : objects)
			include(object.m_position);

		for(const Data::MapSegment::TileMap::DoodadObject& doodad : doodads)
			include(doodad.m_position);

		TP_VERIFY(min.m_x != INT32_MAX, aStructure->m_debugInfo, "Structure '%s' is empty.", aStructure->m_name.c_str());

		std::unique_ptr<Data::MapSegment::TileMap> tileMap = std::make_unique<Data::MapSegment::TileMap>();
		tileMap->m_size = { max.m_x - min.m_x + 1, max.m_y - min.m_y + 1 };
		tileMap->m_tiles.resize((size_t)(tileMap->m_size.m_x * tileMap->m_size.m_y), 0);

		bool hasCoverTiles = false;

		for(int32_t y = 0; y < tileMap->m_size.m_y; y++)
		{
			for(int32_t x = 0; x < tileMap->m_size.m_x; x++)
			{
				size_t from = (size_t)((x + min.m_x) + (y + min.m_y) * width);
				size_t to = (size_t)(x + y * tileMap->m_size.m_x);
				tileMap->m_tiles[to] = tiles[from];

				if(coverTiles[from] != 0)
				{
					if(!hasCoverTiles)
					{
						tileMap->m_coverTiles.resize(tileMap->m_tiles.size(), 0);
						hasCoverTiles = true;
					}

					tileMap->m_coverTiles[to] = coverTiles[from];
				}
			}
		}

		for(Data::MapSegment::Object& object : objects)
		{
			object.m_position = { object.m_position.m_x - min.m_x, object.m_position.m_y - min.m_y };
			tileMap->m_objects.push_back(object);
		}

		for(Data::MapSegment::TileMap::DoodadObject& doodad : doodads)
		{
			doodad.m_position = { doodad.m_position.m_x - min.m_x, doodad.m_position.m_y - min.m_y };
			tileMap->m_doodads.push_back(doodad);
		}

		// Make map segment
		Data::MapSegment* mapSegment = aManifest->GetContainer<Data::MapSegment>()->GetByName(aPersistentIdTable, aStructure->m_name.c_str());
		TP_VERIFY(!mapSegment->m_defined, aStructure->m_debugInfo, "'%s' already defined.", aStructure->m_name.c_str());

		mapSegment->m_debugInfo = aStructure->m_debugInfo;
		mapSegment->m_type = Data::MapSegment::TYPE_PALETTED;
		mapSegment->m_string = aStructure->m_string;
		mapSegment->m_underlyingTileSpriteId = aStructure->m_underlyingTileSpriteId;
		mapSegment->m_tileMap = std::move(tileMap);

		for(const Vec2& position : entrances)
		{
			Data::MapSegment::Entrance entrance;
			entrance.m_position = { position.m_x - min.m_x, position.m_y - min.m_y };
			TP_VERIFY(!mapSegment->m_tileMap->IsSet(entrance.m_position), aStructure->m_debugInfo, "Entrance (%u, %u) in structure '%s' can't be on one of its tiles.",
				aStructure->m_x + (uint32_t)position.m_x, aStructure->m_y + (uint32_t)position.m_y, aStructure->m_name.c_str());
			mapSegment->m_entrances.push_back(entrance);
		}

		for(Data::MapSegment::Anchor& anchor : anchors)
		{
			anchor.m_position = { anchor.m_position.m_x - min.m_x, anchor.m_position.m_y - min.m_y };

			// Default spawns: the structure's own, otherwise the sheet's
			const AnchorSpawn* anchorSpawn = NULL;

			for(const AnchorSpawn& t : aStructure->m_anchorSpawns)
			{
				if(t.m_anchor == anchor.m_name)
					anchorSpawn = &t;
			}

			if(anchorSpawn == NULL)
			{
				for(const AnchorSpawn& t : sheet->m_anchorSpawns)
				{
					if(t.m_anchor == anchor.m_name)
						anchorSpawn = &t;
				}
			}

			if(anchorSpawn != NULL)
				anchor.m_mapEntitySpawnIds = anchorSpawn->m_mapEntitySpawnIds;

			aOutAnchorNames.insert(anchor.m_name);
			mapSegment->m_anchors.push_back(anchor);
		}

		for(const AnchorSpawn& anchorSpawn : aStructure->m_anchorSpawns)
			TP_VERIFY(mapSegment->HasAnchor(anchorSpawn.m_anchor), anchorSpawn.m_debugInfo, "Structure '%s' has no anchor called '%s'.", aStructure->m_name.c_str(), anchorSpawn.m_anchor.c_str());

		mapSegment->m_defined = true;
	}

}
