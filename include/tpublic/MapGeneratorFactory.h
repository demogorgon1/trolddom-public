#pragma once

#include "MapGenerators/Dungeon.h"

#include "MapGeneratorBase.h"

namespace tpublic
{

	class MapGeneratorFactory
	{
	public:
		MapGeneratorFactory()
		{
			_Register<MapGenerators::Dungeon>();
		}

		~MapGeneratorFactory()
		{

		}

		MapGeneratorBase*
		Create(
			uint32_t			aId) const
		{
			if(!m_functions[aId])
				return NULL;

			return m_functions[aId]();
		}

	private:

		std::function<MapGeneratorBase*()>			m_functions[MapGenerator::NUM_IDS];

		template<typename _T>
		void
		_Register()
		{
			m_functions[_T::ID] = []() { return new _T(); };
		}

	};

}
