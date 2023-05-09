#pragma once

#include <kpublic/Image.h>

namespace kpublic
{

	class Manifest;
	class MapData;

	class MapImageOutput
	{
	public:
				MapImageOutput(
					const Manifest*				aManifest);
				~MapImageOutput();

		void	Generate(
					const MapData*				aMap,
					const char*					aPath);

	private:

		const Manifest*		m_manifest;

		typedef std::unordered_map<uint32_t, std::unique_ptr<Image>> TileImageTable;
		TileImageTable		m_tileImageTable;
	};

}