#include "Pcheader.h"

#include <tpublic/Data/Sound.h>

#include <tpublic/Manifest.h>

#include "FileWriter.h"
#include "SoundDataBuilder.h"

namespace tpublic
{

	namespace
	{
		std::string 
		_HashToFileName(
			uint32_t	aHash[5])
		{
			std::string t;
			for(size_t i = 0; i < 5; i++)
			{
				char buffer[9];
				TP_STRING_FORMAT(buffer, sizeof(buffer), "%08X", aHash[i]);
				t += buffer;
			}
			return t;
		}
	}

	//--------------------------------------------------------------------------------

	SoundDataBuilder::SoundDataBuilder()
	{

	}
	
	SoundDataBuilder::~SoundDataBuilder()
	{

	}

	void		
	SoundDataBuilder::Build(
		const Manifest*		aManifest,
		const char*			aOutputPath,
		const char*			aCachePath,
		const char*			aToolsPath)
	{
		if(!std::filesystem::exists(Helpers::Format("%s/opusenc.exe", aToolsPath)))
		{
			printf("WARNING: opusenc.exe not found in %s, not building sound data.", aToolsPath);
			return;
		}

		std::filesystem::create_directories(aCachePath);

		// Load sounds, converting them to opus if needed
		aManifest->GetContainer<tpublic::Data::Sound>()->ForEach([&](
			const tpublic::Data::Sound* aSound)
		{
			std::filesystem::path path = aSound->m_source.c_str();
			std::string extension = path.extension().string();

			if(extension == ".opus")
			{
				_Add(aSound->m_id, aSound->m_source.c_str());
			}
			else if(extension == ".wav")
			{
				uint32_t hash[5];
				if(!Hash::File(aSound->m_source.c_str(), hash))
					TP_CHECK(false, "Failed to load file: %s", aSound->m_source.c_str());

				std::string cacheFilePath = aCachePath;
				cacheFilePath += "/" + _HashToFileName(hash);
				cacheFilePath += ".opus";

				if(!std::filesystem::exists(cacheFilePath))
				{
					printf("============================ %s ============================\n", aSound->m_name.c_str());

					std::string cacheFilePathTemp = cacheFilePath + ".tmp";
					std::string toolsPathString = std::filesystem::path(aToolsPath).lexically_normal().string();

					char opusEncCommand[2048];
					TP_STRING_FORMAT(opusEncCommand, sizeof(opusEncCommand), 
						"%s\\opusenc --discard-comments --discard-pictures --bitrate 64 --downmix-mono %s %s",
						toolsPathString.c_str(), aSound->m_source.c_str(), cacheFilePathTemp.c_str());

					int result = system(opusEncCommand);
					TP_CHECK(result == 0, "Failed to run opusenc.");

					std::error_code ec;
					std::filesystem::rename(cacheFilePathTemp, cacheFilePath, ec);
					TP_CHECK(!ec, "Failed to rename: %s", cacheFilePathTemp.c_str());
				}

				_Add(aSound->m_id, cacheFilePath.c_str());
			}
			else
			{
				TP_CHECK(false, "Unsupported sound format: %s", extension.c_str());
			}

			return true;
		});

		// Write sound data file
		{
			FileWriter file(aOutputPath);
			m_data.ToStream(&file);
		}
	}

	//--------------------------------------------------------------------------------

	void		
	SoundDataBuilder::_Add(
		uint32_t			aSoundId,
		const char*			aPath)
	{	
		FILE* f = fopen(aPath, "rb");
		TP_CHECK(f != NULL, "Failed to open file: %s", aPath);

		bool failed = false;

		try
		{
			fseek(f, 0, SEEK_END);
			size_t fileSize = (size_t)ftell(f);
			fseek(f, 0, SEEK_SET);

			if(fileSize == 0)
			{
				failed = true;
			}
			else
			{
				std::vector<uint8_t> buffer;
				buffer.resize(fileSize);
				size_t readBytes = (size_t)fread(&buffer[0], 1, fileSize, f);
				if(readBytes != fileSize)
				{
					failed = true;
				}
				else
				{
					m_data.AddEntry(aSoundId, buffer);
				}
			}
		}
		catch(...)
		{
			failed = true;
		}

		fclose(f);
		TP_CHECK(!failed, "Failed to add sound file: %s", aPath);
	}

}