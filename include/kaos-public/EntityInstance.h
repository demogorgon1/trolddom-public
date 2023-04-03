#pragma once

#include "ComponentBase.h"
#include "SystemBase.h"

namespace kaos_public
{

	class EntityInstance
	{
	public:
		EntityInstance(
			uint32_t			aEntityInstanceId)
			: m_entityInstanceId(aEntityInstanceId)
		{

		}

		~EntityInstance()
		{

		}

		void
		AddComponent(
			ComponentBase*		aComponent)
		{
			m_components.push_back(std::unique_ptr<ComponentBase>(aComponent));
		}

		void
		AddSystem(
			const SystemBase*	aSystem)
		{
			m_systems.push_back(aSystem);
		}

		template <typename _T>
		_T*
		GetComponent()
		{
			for(std::unique_ptr<ComponentBase>& component : m_components)
			{
				if(component->GetComponentId() == _T::ID)
					return (_T*)component.get();
			}
			return NULL;
		}

		// Data access
		uint32_t		GetEntityInstanceId() const { return m_entityInstanceId; }
		
	private:
		
		uint32_t									m_entityInstanceId;
		std::vector<const SystemBase*>				m_systems;
		std::vector<std::unique_ptr<ComponentBase>>	m_components;
	};

}