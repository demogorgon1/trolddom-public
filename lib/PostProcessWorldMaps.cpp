#include "Pcheader.h"

#include <tpublic/Data/Map.h>
#include <tpublic/Data/Sprite.h>
#include <tpublic/Data/WorldMap.h>

#include <tpublic/Manifest.h>
#include <tpublic/MapData.h>
#include <tpublic/Perlin.h>

namespace tpublic
{

	namespace
	{

		struct WorldMapSource
		{
			void
			AddMap(
				uint32_t			aMapId,
				const MapData*		aMap)
			{
				assert(aMap->m_worldMap);

				m_min.m_x = Base::Min(aMap->m_worldMap->m_offset.m_x, m_min.m_x);
				m_min.m_y = Base::Min(aMap->m_worldMap->m_offset.m_y, m_min.m_y);
				m_max.m_x = Base::Max(aMap->m_worldMap->m_offset.m_x + aMap->m_width, m_max.m_x);
				m_max.m_y = Base::Max(aMap->m_worldMap->m_offset.m_y + aMap->m_height, m_max.m_y);

				m_maps.push_back({ aMapId, aMap });
			}

			void
			Build(
				const Manifest*		aManifest,
				Data::WorldMap*		aWorldMap)
			{
				TP_VERIFY(aWorldMap->m_margins.size() == 4, aWorldMap->m_debugInfo, "Invalid margins.");

				m_min.m_x += aWorldMap->m_margins[0];
				m_min.m_y += aWorldMap->m_margins[1];

				m_max.m_x += aWorldMap->m_margins[2];
				m_max.m_y += aWorldMap->m_margins[3];

				Vec2 size = { m_max.m_x - m_min.m_x, m_max.m_y - m_min.m_y };

				const Data::Sprite* landTileSprite = aManifest->GetById<Data::Sprite>(aWorldMap->m_landTileSpriteId);
				const Data::Sprite* oceanTileSprite = aManifest->GetById<Data::Sprite>(aWorldMap->m_oceanTileSpriteId);

				std::unique_ptr<Image> image = std::make_unique<Image>();
				image->Allocate((uint32_t)size.m_x, (uint32_t)size.m_y);

				std::mt19937 random;
				Perlin::Seed noise;
				Perlin::InitSeed(&noise, random);

				// Build full image
				for(const std::pair<uint32_t, const MapData*>& t : m_maps)
				{
					const MapData* map = t.second;
					assert(map->m_worldMap);

					Image source;
					source.LoadPNG(map->m_worldMap->m_sourcePath.c_str());

					std::unique_ptr<Data::WorldMap::SubMap> subMap = std::make_unique<Data::WorldMap::SubMap>();
					subMap->m_mapId = t.first;

					Vec2 position = { map->m_worldMap->m_offset.m_x - m_min.m_x, map->m_worldMap->m_offset.m_y - m_min.m_y };

					std::unordered_set<Vec2, Vec2::Hasher> coast;

					// Make outline
					{
						std::function<bool(int32_t, int32_t)> landmass = [&, p = source.GetData()](
							int32_t aX,
							int32_t aY) -> bool
						{
							int32_t x = Base::Min((int32_t)source.GetWidth() - 1, Base::Max(0, aX));
							int32_t y = Base::Min((int32_t)source.GetHeight() - 1, Base::Max(0, aY));
							return p[x + y * (int32_t)source.GetWidth()].m_r != 0;
						};

						std::vector<Vec2> outline;
						Vec2 outlineSum;
						int32_t outlineSumCount = 0;

						for (int32_t y = 0; y < (int32_t)source.GetHeight(); y++)
						{
							for (int32_t x = 0; x < (int32_t)source.GetWidth(); x++)
							{
								if(landmass(x, y) &&
									(!landmass(x - 1, y) || !landmass(x + 1, y) || !landmass(x, y - 1) || !landmass(x, y + 1)))
								{
									outline.push_back({ position.m_x + x, position.m_y + y });
									coast.insert({ position.m_x + x, position.m_y + y });

									outlineSum.m_x += position.m_x + x;
									outlineSum.m_y += position.m_y + y;

									outlineSumCount++;
								}
							}
						}

						subMap->m_outline = std::move(outline);

						if(outlineSumCount > 0)
							subMap->m_center = { outlineSum.m_x / outlineSumCount, outlineSum.m_y / outlineSumCount };

						subMap->m_min = position;
						subMap->m_max.m_x = position.m_x + (int32_t)source.GetWidth();
						subMap->m_max.m_y = position.m_y + (int32_t)source.GetHeight();
					}

					// Insert in full image
					image->Insert((uint32_t)position.m_x, (uint32_t)position.m_y, source);

					aWorldMap->m_subMaps.push_back(std::move(subMap));
				}

				// Convert to color (sepia)
				{
					std::function<bool(int32_t, int32_t)> landmass = [&, p = image->GetData()](
						int32_t aX,
						int32_t aY) -> bool
					{
						int32_t x = Base::Min((int32_t)image->GetWidth() - 1, Base::Max(0, aX));
						int32_t y = Base::Min((int32_t)image->GetHeight() - 1, Base::Max(0, aY));
						return p[x + y * (int32_t)image->GetWidth()].m_r != 0;
					};

					std::unique_ptr<Image> tmp = std::make_unique<Image>();
					tmp->Allocate(image->GetWidth(), image->GetHeight());

					for(int32_t y = 0; y < size.m_y; y++)
					{
						float vBright = 1.0f;

						if(y < 32)
							vBright = Helpers::SmoothStep(0.0f, 32.0f, (float)y);
						else if(y > size.m_y - 32)
							vBright = Helpers::SmoothStep(0.0f, 32.0f, (float)(size.m_y - y));

						for (int32_t x = 0; x < size.m_x; x++)
						{
							bool isLand = landmass(x, y);
							const Data::Sprite* tileSprite = isLand ? landTileSprite : oceanTileSprite;
							Image::RGBA color = tileSprite->m_info.m_averageColor;

							static const int32_t RADIUS = 10;
							uint32_t nearbyLand = 0;
							uint32_t nearbyOcean = 0;

							for(int32_t iy = -RADIUS; iy <= RADIUS; iy += 2)
							{
								for (int32_t ix = -RADIUS; ix <= RADIUS; ix += 2)								
								{
									if(ix * ix + iy * iy <= RADIUS * RADIUS)
									{
										if(landmass(x + ix, y + iy))
											nearbyLand++;
										else
											nearbyOcean++;
									}
								}
							}

							float landProportion = (float)nearbyLand / (float)(nearbyLand + nearbyOcean);
							float brightness = 0.0f;							

							if(!isLand)
							{
								brightness = 1.0f - Helpers::SmoothStep(0.0f, 1.0f, landProportion);
							}
							else
							{
								brightness = 1.0f;
								float noiseSample1 = Helpers::Clamp((float)Perlin::Sample(x * 1000, y * 1800, 0, &noise) / 64000.0f + 0.5f, 0.0f, 1.0f);
								float noiseSample2 = Helpers::Clamp((float)Perlin::Sample(x * 900, y * 1400, 0, &noise) / 49000.0f + 0.5f, 0.0f, 1.0f);
								float noiseSample3 = Helpers::Clamp((float)Perlin::Sample(x * 1500, y * 2700, 0, &noise) / 55000.0f + 0.5f, 0.0f, 1.0f);
								float noiseSample4 = ((float)(random() >> 24) / 255.0f);
								float n = noiseSample1 * noiseSample2 * noiseSample3 * 0.8f + noiseSample4 * 0.2f;
								brightness *= 0.95f + n * 0.05f;
							}

							tmp->GetData()[x + y * size.m_x] = Image::Sepia(color, brightness * vBright);
						}
					}

					aWorldMap->m_image = std::move(tmp);
				}
			}

			// Public data
			std::vector<std::pair<uint32_t, const MapData*>>	m_maps;
			Vec2												m_min;
			Vec2												m_max;
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

			aManifest->GetContainer<Data::Map>()->ForEach([&worldMapSources](
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

					t->AddMap(aMap->m_id, aMap->m_data.get());
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