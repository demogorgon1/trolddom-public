#pragma once

#include "MapGeneratorBase.h"

namespace tpublic
{

	class MapGeneratorFactory
	{
	public:
								MapGeneratorFactory();
								~MapGeneratorFactory();

		MapGeneratorBase*		Create(
									uint32_t			aId) const;

	private:

		std::function<MapGeneratorBase*()>		m_functions[MapGenerator::NUM_IDS];

		template<typename _T>
		void
		_Register()
		{
			m_functions[_T::ID] = []() { return new _T(); };
		}

	};

}
