#pragma once

namespace tpublic
{

	class TaggedData;

	class MapGeneratorRuntime
	{
	public:
		MapGeneratorRuntime(
			const Manifest*			aManifest);
		~MapGeneratorRuntime();

		// Public data		
		struct Internal;
		std::unique_ptr<Internal>	m_internal;

		TaggedData*					m_entities;
	};

}