#include "Pcheader.h"

#include <tpublic/Image.h>

#include <algorithm>
#include <fstream>
#include <string_view>

#include "ExportMapXCF.h"
#include "XCF.h"

namespace tpublic_util
{

	namespace
	{

		static const size_t MAX_LISTED_PIXELS = 10;

		enum Usage
		{
			USAGE_NONE,
			USAGE_WORLD_MAP,
			USAGE_LAYER
		};

		struct Pixel
		{
			uint32_t					m_x = 0;
			uint32_t					m_y = 0;
			tpublic::Image::RGBA		m_xcf;
			tpublic::Image::RGBA		m_png;
		};

		struct Difference
		{
			bool						m_sizeMismatch = false;
			size_t						m_count = 0;
			std::vector<Pixel>			m_listed;
		};

		std::string
		_GetKey(
			const std::string&				aText)
		{
			// Keys look like "source:", "layers:" or "world_map main:", possibly on the line before a bracket
			size_t end = aText.find_last_not_of(" \t\r\n");
			if(end == std::string::npos)
				return std::string();

			size_t lineBegin = aText.find_last_of('\n', end);
			lineBegin = lineBegin == std::string::npos ? 0 : lineBegin + 1;

			size_t keyBegin = aText.find_first_not_of(" \t", lineBegin);
			size_t keyEnd = aText.find_first_of(" \t:", keyBegin);
			if(keyEnd == std::string::npos || keyEnd > end + 1)
				keyEnd = end + 1;

			return aText.substr(keyBegin, keyEnd - keyBegin);
		}

		bool
		_EndsWith(
			const std::vector<std::string>&					aBlocks,
			const std::initializer_list<const char*>&		aSuffix)
		{
			if(aBlocks.size() < aSuffix.size())
				return false;

			size_t i = aBlocks.size() - aSuffix.size();
			for(const char* key : aSuffix)
			{
				if(aBlocks[i++] != key)
					return false;
			}
			return true;
		}

		Usage
		_GetUsage(
			const std::vector<std::string>&	aBlocks,
			const std::string&				aKey)
		{
			// Only these are drawn in map .xcf files:
			//
			// map <name>: { layers: [ { source: "<png>" } ] }
			// map <name>: { world_map <name>: { source: "<png>" } }
			if(aKey != "source")
				return USAGE_NONE;

			if(_EndsWith(aBlocks, { "map", "layers", "" }))
				return USAGE_LAYER;

			if(_EndsWith(aBlocks, { "map", "world_map" }))
				return USAGE_WORLD_MAP;

			return USAGE_NONE;
		}

		void
		_GetReferencedPNGFiles(
			const std::filesystem::path&	aDirectory,
			const std::string&				aPrefix,
			std::map<std::string, Usage>&	aOut)
		{
			for(const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(aDirectory))
			{
				if(!entry.is_regular_file() || entry.path().extension() != ".txt")
					continue;

				std::ifstream file(entry.path(), std::ios::binary);
				std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

				// Look for map layer and world map sources like "<prefix><layer name>.png", skipping comments. Other
				// PNG files can be referenced as well (sprites etc), so keep track of which blocks we're in.
				std::vector<std::string> blocks;
				std::string sinceLastToken;
				size_t offset = 0;
				while(offset < text.size())
				{
					char c = text[offset];

					if(c == '#')
					{
						size_t end = text.find('\n', offset);
						offset = end == std::string::npos ? text.size() : end;
					}
					else if(c == '"')
					{
						size_t end = text.find('"', offset + 1);
						if(end == std::string::npos)
							break;

						std::string_view s(&text[offset + 1], end - offset - 1);
						Usage usage = _GetUsage(blocks, _GetKey(sinceLastToken));

						if(usage != USAGE_NONE && s.size() > aPrefix.size() + 4 && s.starts_with(aPrefix) && s.ends_with(".png") && s.find_first_of("/\\") == std::string_view::npos)
						{
							Usage& existing = aOut.insert({ std::string(s), usage }).first->second;
							existing = (std::max)(existing, usage);
						}

						sinceLastToken.clear();
						offset = end + 1;
					}
					else if(c == '{' || c == '[')
					{
						blocks.push_back(_GetKey(sinceLastToken));
						sinceLastToken.clear();
						offset++;
					}
					else if(c == '}' || c == ']')
					{
						if(!blocks.empty())
							blocks.pop_back();
						sinceLastToken.clear();
						offset++;
					}
					else
					{
						sinceLastToken.push_back(c);
						offset++;
					}
				}
			}
		}

		void
		_ClearTransparentPixels(
			tpublic::Image&					aImage)
		{
			// GIMP stores colors of fully transparent pixels, but doesn't export them to PNG files by default. World
			// maps look at colors regardless of alpha, so these must be cleared.
			tpublic::Image::RGBA* p = aImage.GetData();
			size_t count = (size_t)aImage.GetWidth() * (size_t)aImage.GetHeight();
			for(size_t i = 0; i < count; i++)
			{
				if(p[i].m_a == 0)
					p[i] = tpublic::Image::RGBA(0, 0, 0, 0);
			}
		}

		std::string
		_ColorToString(
			const tpublic::Image::RGBA&		aColor)
		{
			if(aColor == tpublic::Image::RGBA(0, 0, 0, 0))
				return "transparent";

			char buffer[64];
			snprintf(buffer, sizeof(buffer), "%u,%u,%u,%u", aColor.m_r, aColor.m_g, aColor.m_b, aColor.m_a);
			return buffer;
		}

		size_t
		_GetInvalidAlphaPixels(
			const tpublic::Image&			aImage,
			std::vector<Pixel>&				aOutListed)
		{
			const tpublic::Image::RGBA* p = aImage.GetData();
			size_t count = 0;

			for(uint32_t y = 0; y < aImage.GetHeight(); y++)
			{
				for(uint32_t x = 0; x < aImage.GetWidth(); x++)
				{
					const tpublic::Image::RGBA& pixel = p[(size_t)x + (size_t)y * (size_t)aImage.GetWidth()];
					if(pixel.m_a == 0 || pixel.m_a == 255)
						continue;

					count++;

					if(aOutListed.size() < MAX_LISTED_PIXELS)
						aOutListed.push_back({ x, y, pixel, tpublic::Image::RGBA() });
				}
			}

			return count;
		}

		Difference
		_Compare(
			const tpublic::Image&			aXCF,
			const tpublic::Image&			aPNG)
		{
			Difference difference;

			if(aXCF.GetWidth() != aPNG.GetWidth() || aXCF.GetHeight() != aPNG.GetHeight())
			{
				difference.m_sizeMismatch = true;
				return difference;
			}

			const tpublic::Image::RGBA* xcf = aXCF.GetData();
			const tpublic::Image::RGBA* png = aPNG.GetData();

			for(uint32_t y = 0; y < aXCF.GetHeight(); y++)
			{
				for(uint32_t x = 0; x < aXCF.GetWidth(); x++)
				{
					size_t i = (size_t)x + (size_t)y * (size_t)aXCF.GetWidth();
					if(xcf[i] == png[i])
						continue;

					difference.m_count++;

					if(difference.m_listed.size() < MAX_LISTED_PIXELS)
						difference.m_listed.push_back({ x, y, xcf[i], png[i] });
				}
			}

			return difference;
		}

		void
		_PrintDifference(
			const Difference&				aDifference,
			const tpublic::Image&			aXCF,
			const tpublic::Image&			aPNG)
		{
			if(aDifference.m_sizeMismatch)
			{
				printf("  size: xcf %ux%u, png %ux%u\n", aXCF.GetWidth(), aXCF.GetHeight(), aPNG.GetWidth(), aPNG.GetHeight());
				return;
			}

			for(const Pixel& pixel : aDifference.m_listed)
				printf("  (%u,%u): xcf %s, png %s\n", pixel.m_x, pixel.m_y, _ColorToString(pixel.m_xcf).c_str(), _ColorToString(pixel.m_png).c_str());

			if(aDifference.m_count > aDifference.m_listed.size())
				printf("  ... and %zu more\n", aDifference.m_count - aDifference.m_listed.size());
		}

		void
		_ExportMapXCFFile(
			const std::filesystem::path&	aPath,
			bool							aCheckOnly,
			ExportMapXCFResult&				aResult)
		{
			std::string name = aPath.filename().string();
			std::string stem = aPath.stem().string();
			std::string prefix = stem + "-";
			std::filesystem::path directory = aPath.parent_path();
			if(directory.empty())
				directory = ".";

			std::map<std::string, Usage> pngFiles;
			_GetReferencedPNGFiles(directory, prefix, pngFiles);

			if(pngFiles.empty())
			{
				printf("%s: no map layers referenced\n", name.c_str());
				return;
			}

			XCF xcf(aPath.lexically_normal().string().c_str());

			for(const std::pair<const std::string, Usage>& pngFileAndUsage : pngFiles)
			{
				const std::string& pngFile = pngFileAndUsage.first;
				std::string layerName = pngFile.substr(prefix.size(), pngFile.size() - prefix.size() - 4);
				std::filesystem::path pngPath = directory / pngFile;

				const XCF::Layer* layer = xcf.GetLayerByName(layerName.c_str());
				if(layer == NULL)
				{
					printf("%s: warning: %s is referenced, but there is no layer called '%s'\n", name.c_str(), pngFile.c_str(), layerName.c_str());
					continue;
				}

				tpublic::Image image;
				xcf.ReadLayerImage(layer, image);
				_ClearTransparentPixels(image);

				if(pngFileAndUsage.second == USAGE_LAYER)
				{
					// Map layer pixels must be either fully transparent or fully opaque
					std::vector<Pixel> listed;
					size_t count = _GetInvalidAlphaPixels(image, listed);
					if(count > 0)
					{
						aResult.m_numErrors++;

						printf("%s: %s -> %s (ERROR: %zu %s alpha other than 0 or 255)\n", name.c_str(), layerName.c_str(), pngFile.c_str(), count, count == 1 ? "pixel has" : "pixels have");

						for(const Pixel& pixel : listed)
							printf("  (%u,%u): xcf %s\n", pixel.m_x, pixel.m_y, _ColorToString(pixel.m_xcf).c_str());

						if(count > listed.size())
							printf("  ... and %zu more\n", count - listed.size());

						continue;
					}
				}

				bool pngExists = std::filesystem::exists(pngPath);
				tpublic::Image existing;
				Difference difference;

				if(pngExists)
				{
					existing.LoadPNG(pngPath.string().c_str());
					difference = _Compare(image, existing);

					if(!difference.m_sizeMismatch && difference.m_count == 0)
					{
						printf("%s: %s -> %s (unchanged)\n", name.c_str(), layerName.c_str(), pngFile.c_str());
						continue;
					}
				}

				aResult.m_numChanged++;

				std::string summary;
				if(!pngExists)
					summary = "png missing";
				else if(difference.m_sizeMismatch)
					summary = "size differs";
				else
					summary = std::to_string(difference.m_count) + (difference.m_count == 1 ? " pixel differs" : " pixels differ");

				if(aCheckOnly)
				{
					printf("%s: %s -> %s (DIFFERS: %s)\n", name.c_str(), layerName.c_str(), pngFile.c_str(), summary.c_str());
				}
				else
				{
					image.SavePNG(pngPath.string().c_str());
					printf("%s: %s -> %s (updated: %s)\n", name.c_str(), layerName.c_str(), pngFile.c_str(), summary.c_str());
				}

				if(pngExists)
					_PrintDifference(difference, image, existing);
			}
		}

	}

	//---------------------------------------------------------------------------------

	ExportMapXCFResult
	ExportMapXCF(
		const std::vector<std::string>&		aPaths,
		bool								aCheckOnly)
	{
		ExportMapXCFResult result;

		for(const std::string& path : aPaths)
		{
			TP_CHECK(std::filesystem::exists(path), "Path not found: %s", path.c_str());

			if(std::filesystem::is_directory(path))
			{
				std::vector<std::filesystem::path> xcfPaths;
				for(const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(path))
				{
					if(entry.is_regular_file() && entry.path().extension() == ".xcf")
						xcfPaths.push_back(entry.path());
				}

				std::sort(xcfPaths.begin(), xcfPaths.end());

				for(const std::filesystem::path& xcfPath : xcfPaths)
					_ExportMapXCFFile(xcfPath, aCheckOnly, result);
			}
			else
			{
				_ExportMapXCFFile(path, aCheckOnly, result);
			}
		}

		return result;
	}

}
