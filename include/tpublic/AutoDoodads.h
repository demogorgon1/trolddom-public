#pragma once

#include "Vec2.h"

namespace tpublic
{

	namespace Data
	{
		struct Doodad;
	}

	class Manifest;

	class AutoDoodads
	{
	public:
		typedef std::function<void(const Vec2&, uint32_t, uint32_t)> Callback;

				AutoDoodads(
					const Manifest*										aManifest);
				~AutoDoodads();

		void	GenerateDoodads(
					uint32_t											aSeed,
					const uint32_t*										aTileMap,
					const Vec2&											aSize,
					const Vec2&											aRegionPosition,
					const Vec2&											aRegionSize,					
					std::unordered_map<Vec2, uint32_t, Vec2::Hasher>&	aOutCoverageMap,
					Callback											aCallback) const;

	private:

		typedef std::unordered_map<uint32_t, std::unique_ptr<std::vector<const Data::Doodad*>>> TileTable;
		TileTable						m_tileTable;

		void								_InitDoodad(
												const Data::Doodad*									aDoodad);
		std::vector<const Data::Doodad*>*	_GetOrCreateTileEntry(
												uint32_t											aTileSpriteId);
	};

}