#pragma once

#include "Components/Combat.h"
#include "Components/NPC.h"
#include "Components/Position.h"

#include "Component.h"

namespace kaos_public
{

	class ComponentFactory
	{
	public:
		ComponentFactory()
		{
			_Register<Components::Combat>();
			_Register<Components::NPC>();
			_Register<Components::Position>();
		}

		~ComponentFactory()
		{

		}

		ComponentBase*
		Create(
			uint32_t			aId) 
		{
			if(!m_functions[aId])
				return NULL;

			return m_functions[aId]();
		}

	private:

		std::function<ComponentBase*()>				m_functions[Component::NUM_IDS];

		template<typename _T>
		void
		_Register()
		{
			m_functions[_T::ID] = []() { return new _T(); };
		}

	};

}