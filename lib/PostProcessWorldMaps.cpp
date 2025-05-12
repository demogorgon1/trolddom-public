#include "Pcheader.h"

#include <tpublic/Data/Map.h>
#include <tpublic/Data/Sprite.h>
#include <tpublic/Data/WorldMap.h>

#include <tpublic/Manifest.h>
#include <tpublic/MapData.h>

namespace tpublic
{

	namespace
	{

		struct WorldMapSource
		{
			void
			AddMap(
				const MapData*		aMap)
			{
				assert(aMap->m_worldMap);

				m_min.m_x = Base::Min(aMap->m_worldMap->m_offset.m_x, m_min.m_x);
				m_min.m_y = Base::Min(aMap->m_worldMap->m_offset.m_y, m_min.m_y);
				m_max.m_x = Base::Max(aMap->m_worldMap->m_offset.m_x + aMap->m_width, m_max.m_x);
				m_max.m_y = Base::Max(aMap->m_worldMap->m_offset.m_y + aMap->m_height, m_max.m_y);

				m_maps.push_back(aMap);
			}

			void
			Build(
				const Manifest*		aManifest,
				Data::WorldMap*		aWorldMap)
			{
				Vec2 size = { m_max.m_x - m_min.m_x, m_max.m_y - m_min.m_y };

				const Data::Sprite* landTileSprite = aManifest->GetById<Data::Sprite>(aWorldMap->m_landTileSpriteId);
				const Data::Sprite* oceanTileSprite = aManifest->GetById<Data::Sprite>(aWorldMap->m_oceanTileSpriteId);

				Image image;
				image.Allocate((uint32_t)size.m_x, (uint32_t)size.m_y);

				for(const MapData* map : m_maps)
				{
					assert(map->m_worldMap);

					Image source;
					source.LoadPNG(map->m_worldMap->m_sourcePath.c_str());

					uint32_t x = (uint32_t)(map->m_worldMap->m_offset.m_x - m_min.m_x);
					uint32_t y = (uint32_t)(map->m_worldMap->m_offset.m_y - m_min.m_y);

					image.Insert(x, y, source);
				}

				{
					Image::RGBA* p = image.GetData();

					for(int32_t y = 0; y < size.m_y; y++)
					{
						for (int32_t x = 0; x < size.m_x; x++)
						{
							const Data::Sprite* tileSprite = p->m_r != 0 ? landTileSprite : oceanTileSprite;
							*p = tileSprite->m_info.m_averageColor;
							p++;															
						}
					}
				}			

				image.SavePNG("worldmap.png");
			}

			// Public data
			std::vector<const MapData*>				m_maps;
			Vec2									m_min;
			Vec2									m_max;
		};

	}

	namespace PostProcessWorldMaps
	{

		void		
		Run(
			Manifest* aManifest)
		{
			typedef std::unordered_map<uint32_t, std::unique_ptr<WorldMapSource>> WorldMapSources;
			WorldMapSources worldMapSources;

			aManifest->GetContainer<Data::Map>()->ForEach([aManifest, &worldMapSources](
				const Data::Map* aMap)
			{
				if(aMap->m_data->m_worldMap)
				{
					WorldMapSource* t = NULL;
					WorldMapSources::iterator i = worldMapSources.find(aMap->m_data->m_worldMap->m_worldMapId);
					if(i != worldMapSources.end())
						t = i->second.get();
					else
						worldMapSources[aMap->m_data->m_worldMap->m_worldMapId] = std::unique_ptr<WorldMapSource>(t = new WorldMapSource());

					t->AddMap(aMap->m_data.get());
				}
				return true;
			});

			aManifest->GetContainer<Data::WorldMap>()->ForEach([aManifest, &worldMapSources](
				Data::WorldMap* aWorldMap)
			{
				WorldMapSources::iterator i = worldMapSources.find(aWorldMap->m_id);
				if(i != worldMapSources.end())
					i->second->Build(aManifest, aWorldMap);
				return true;
			});
		}

	}

}