#pragma once

#include "ComponentBase.h"
#include "SystemBase.h"

namespace kaos_public
{

	class EntityInstance
	{
	public:
		EntityInstance(
			uint32_t			aEntityId,
			uint32_t			aEntityInstanceId)
			: m_entityInstanceId(aEntityInstanceId)
			, m_entityId(aEntityId)
		{

		}

		~EntityInstance()
		{

		}

		void
		AddComponent(
			ComponentBase*		aComponent)
		{
			// FIXME: this kinda defeats much of the purpose of using ECS
			m_components.push_back(std::unique_ptr<ComponentBase>(aComponent));
		}

		void
		AddSystem(
			const SystemBase*	aSystem)
		{
			m_systems.push_back(aSystem);
		}

		void
		Serialize(
			IWriter*			aWriter) const
		{
			for(const std::unique_ptr<ComponentBase>& component : m_components)
				component->ToStream(aWriter);
		}

		bool
		Deserialize(
			IReader*			aReader) 
		{
			for (const std::unique_ptr<ComponentBase>& component : m_components)
			{
				if(!component->FromStream(aReader))
					return false;
			}
			return true;
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

		template <typename _T>
		const _T*
		GetComponent() const
		{
			for(const std::unique_ptr<ComponentBase>& component : m_components)
			{
				if(component->GetComponentId() == _T::ID)
					return (const _T*)component.get();
			}
			return NULL;
		}

		// Data access
		uint32_t		GetEntityInstanceId() const { return m_entityInstanceId; }
		uint32_t		GetEntityId() const { return m_entityId; }
		
	private:
		
		uint32_t									m_entityId;
		uint32_t									m_entityInstanceId;
		std::vector<const SystemBase*>				m_systems;
		std::vector<std::unique_ptr<ComponentBase>>	m_components;
	};

}