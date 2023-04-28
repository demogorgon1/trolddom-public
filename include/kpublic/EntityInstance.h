#pragma once

#include "ComponentBase.h"
#include "DataErrorHandling.h"
#include "EntityState.h"

namespace kpublic
{

	class EntityInstance
	{
	public:
		EntityInstance(
			uint32_t				aEntityId,
			uint32_t				aEntityInstanceId)
			: m_entityInstanceId(aEntityInstanceId)
			, m_entityId(aEntityId)
			, m_state(EntityState::INVALID_ID)
		{

		}

		~EntityInstance()
		{
		}

		void
		SetState(
			EntityState::Id			aState,
			uint32_t				aTick)
		{
			m_state = aState;
			m_stateTick = aTick;
		}

		void
		AddComponent(
			ComponentBase*			aComponent)
		{
			// FIXME: this kinda defeats much of the purpose of using ECS
			m_components.push_back(std::unique_ptr<ComponentBase>(aComponent));
		}
		
		void
		SerializeAll(
			IWriter*				aWriter) const
		{
			aWriter->WritePOD(m_state);

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
			aWriter->WritePOD(m_state);

			uint32_t i = 0;
			for(const std::unique_ptr<ComponentBase>& component : m_components)
			{
				if(component->GetFlags() & ComponentBase::FLAG_REPLICATE_TO_OTHERS)
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
			aWriter->WritePOD(m_state);

			uint32_t i = 0;
			for(const std::unique_ptr<ComponentBase>& component : m_components)
			{				
				if(component->GetFlags() & ComponentBase::FLAG_REPLICATE_TO_OWNER)
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
			if(!aReader->ReadPOD(m_state))
				return false;

			while(!aReader->IsEnd())
			{
				uint32_t index;
				if(!aReader->ReadUInt(index))
					return false;

				if((size_t)index >= m_components.size())
					return false;

				if(aOutUpdatedComponents != NULL)
					aOutUpdatedComponents->push_back(m_components[index].get());

				if(!m_components[index])
					return false;

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
				if(component && component->GetComponentId() == _T::ID)
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
				if(component && component->GetComponentId() == _T::ID)
					return (const _T*)component.get();
			}
			return NULL;
		}

		ComponentBase*
		GetComponentBase(
			uint32_t							aComponentId)
		{
			for (std::unique_ptr<ComponentBase>& component : m_components)
			{
				if(component && component->GetComponentId() == aComponentId)
					return component.get();
			}
			return NULL;
		}

		// Data access
		uint32_t										GetEntityInstanceId() const { return m_entityInstanceId; }
		uint32_t										GetEntityId() const { return m_entityId; }
		EntityState::Id									GetState() const { return m_state; }
		bool											IsPlayer() const { return m_entityId == 0; }
		std::vector<std::unique_ptr<ComponentBase>>&	GetComponents() { return m_components; }
		
	private:
		
		uint32_t									m_entityId = 0;
		uint32_t									m_entityInstanceId = 0;
		EntityState::Id								m_state = EntityState::ID_DEFAULT;
		uint32_t									m_stateTick = 0;
		std::vector<std::unique_ptr<ComponentBase>>	m_components;
	};

}