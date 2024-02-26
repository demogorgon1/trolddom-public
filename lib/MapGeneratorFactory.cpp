#include "Pcheader.h"

#include <tpublic/MapGenerators/Dungeon.h>
#include <tpublic/MapGenerators/World.h>

#include <tpublic/MapGeneratorFactory.h>

namespace tpublic
{

	MapGeneratorFactory::MapGeneratorFactory()
	{
		_Register<MapGenerators::Dungeon>();
		_Register<MapGenerators::World>();
	}

	MapGeneratorFactory::~MapGeneratorFactory()
	{

	}

	MapGeneratorBase*
	MapGeneratorFactory::Create(
		uint32_t			aId) const
	{
		if(!m_functions[aId])
			return NULL;

		return m_functions[aId]();
	}

}
