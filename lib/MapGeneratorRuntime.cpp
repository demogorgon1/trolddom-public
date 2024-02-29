#include "Pcheader.h"

#include <tpublic/Data/Entity.h>

#include <tpublic/MapGeneratorRuntime.h>
#include <tpublic/TaggedDataCache.h>

namespace tpublic
{

	struct MapGeneratorRuntime::Internal
	{
		Internal(
			const Manifest*			aManifest)
			: m_entites(aManifest)
		{

		}

		TaggedDataCache<Data::Entity>	m_entites;
	};

	//----------------------------------------------------------------------------------

	MapGeneratorRuntime::MapGeneratorRuntime(
		const Manifest*				aManifest)
	{
		m_internal = std::make_unique<Internal>(aManifest);

		m_entities = m_internal->m_entites.Get();
	}

	MapGeneratorRuntime::~MapGeneratorRuntime()
	{

	}

}