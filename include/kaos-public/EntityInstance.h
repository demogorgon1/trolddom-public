#pragma once

#include "ComponentBase.h"
#include "SystemBase.h"

namespace kaos_public
{

	class EntityInstance
	{
	public:
		EntityInstance()
		{

		}

		~EntityInstance()
		{

		}
		
	private:
		
		std::vector<const SystemBase*>				m_systems;
		std::vector<std::unique_ptr<ComponentBase>>	m_components;
	};

}