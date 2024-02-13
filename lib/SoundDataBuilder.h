#pragma once

#include <tpublic/SoundData.h>

namespace tpublic
{

	class Manifest;

	class SoundDataBuilder
	{
	public:
					SoundDataBuilder();
					~SoundDataBuilder();

		void		Build(
						const Manifest*		aManifest,
						const char*			aOutputPath,
						const char*			aCachePath,
						const char*			aToolsPath);

	private:

		SoundData	m_data;

		void		_Add(
						uint32_t			aSoundId,
						const char*			aPath);
	};

}