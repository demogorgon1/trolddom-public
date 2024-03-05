#pragma once

namespace tpublic
{
	
	namespace Data
	{
		struct Doodad;
	}

	struct Vec2;

	namespace DoodadPlacement
	{
		
		void			AddToCoverageMap(
							const Data::Doodad*										aDoodad,
							const Vec2&												aDoodadPosition,
							std::unordered_map<Vec2, uint32_t, Vec2::Hasher>&		aCoverageMap);
		bool			Check(
							const uint32_t*											aMap,
							const Vec2&												aMapSize,
							const Data::Doodad*										aDoodad,
							const Vec2&												aDoodadPosition,
							const std::unordered_map<Vec2, uint32_t, Vec2::Hasher>&	aCoverageMap);

	}

}