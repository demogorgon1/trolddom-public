#pragma once

#include "Systems/Combat.h"
#include "Systems/NPC.h"

#include "System.h"

namespace kaos_public
{

	class SystemFactory
	{
	public:
		SystemFactory()
		{
			_Register<Systems::Combat>();
			_Register<Systems::NPC>();
		}

		~SystemFactory()
		{

		}

		SystemBase*
		Create(
			uint32_t		aId) 
		{
			if(!m_functions[aId])
				return NULL;

			return m_functions[aId]();
		}

	private:

		std::function<SystemBase*()>	m_functions[System::NUM_IDS];

		template<typename _T>
		void
		_Register()
		{
			m_functions[_T::ID] = []() { return new _T(); };
		}

	};

}