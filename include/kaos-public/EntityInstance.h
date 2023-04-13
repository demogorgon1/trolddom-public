#pragma once

#include "ComponentBase.h"
#include "DataErrorHandling.h"

namespace kaos_public
{

	class EntityInstance
	{
	public:
		EntityInstance(
			uint32_t				aEntityId,
			uint32_t				aEntityInstanceId)
			: m_entityInstanceId(aEntityInstanceId)
			, m_entityId(aEntityId)
		{

		}

		~EntityInstance()
		{

		}

		void
		AddComponent(
			ComponentBase*			aComponent)
		{
			assert(aComponent != NULL);
			
			// FIXME: this kinda defeats much of the purpose of using ECS
			m_components.push_back(std::unique_ptr<ComponentBase>(aComponent));
		}
		
		void
		SerializeAll(
			IWriter*				aWriter) const
		{
			uint32_t i = 0;
			for(const std::unique_ptr<ComponentBase>& component : m_components)
			{
				aWriter->WriteUInt(i);
				component->ToStream(aWriter);

				i++;
			}
		}

		void
		SerializePublic(
			IWriter*				aWriter) const
		{
			uint32_t i = 0;
			for(const std::unique_ptr<ComponentBase>& component : m_components)
			{
				if(component->GetFlags() & ComponentBase::FLAG_PUBLIC)
				{
					aWriter->WriteUInt(i);
					component->ToStream(aWriter);
				}
				i++;
			}
		}

		void
		SerializePrivate(
			IWriter*				aWriter) const
		{
			uint32_t i = 0;
			for(const std::unique_ptr<ComponentBase>& component : m_components)
			{				
				if(component->GetFlags() & ComponentBase::FLAG_PRIVATE)
				{
					aWriter->WriteUInt(i);
					component->ToStream(aWriter);
				}
				i++;
			}
		}

		bool
		Deserialize(
			IReader*							aReader,
			std::vector<const ComponentBase*>*	aOutUpdatedComponents) 
		{
			while(!aReader->IsEnd())
			{
				uint32_t index;
				if(!aReader->ReadUInt(index))
					return false;

				if((size_t)index >= m_components.size())
					return false;

				if(aOutUpdatedComponents != NULL)
					aOutUpdatedComponents->push_back(m_components[index].get());

				if(!m_components[index]->FromStream(aReader))
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
		std::vector<std::unique_ptr<ComponentBase>>	m_components;
	};

}