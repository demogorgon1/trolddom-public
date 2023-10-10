#pragma once

#include "Component.h"
#include "ComponentBase.h"

namespace tpublic
{

	class ComponentManager
	{
	public:
							ComponentManager();
							~ComponentManager();

		ComponentBase*		Create(
								uint32_t			aId) const;

	private:

		std::function<ComponentBase*()>		m_functions[Component::NUM_IDS];

		template<typename _T>
		void
		_Register()
		{
			m_functions[_T::ID] = []() { return new _T(); };
		}
	};

}