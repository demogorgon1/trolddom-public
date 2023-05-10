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
					const char*					aDataPath,
					const Manifest*				aManifest);
				~MapImageOutput();

		void	Generate(
					const MapData*				aMap,
					const char*					aPath);

	private:

		std::string			m_dataPath;
		const Manifest*		m_manifest;

		typedef std::unordered_map<uint32_t, std::unique_ptr<Image>> TileImageTable;
		TileImageTable		m_tileImageTable;
	};

}