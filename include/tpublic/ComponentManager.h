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

		struct ComponentType
		{
			std::function<ComponentBase*()>		m_create;
			ComponentSchema						m_schema;
		};

		ComponentType							m_componentTypes[Component::NUM_IDS];

		template<typename _T>
		void
		_Register()
		{
			ComponentType& t = m_componentTypes[_T::ID];

			_T::CreateSchema(&t.m_schema);

			t.m_create = []() { return new _T(); };
		}
	};

}