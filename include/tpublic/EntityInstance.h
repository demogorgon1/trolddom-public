#pragma once

#include "ComponentBase.h"
#include "ComponentManager.h"
#include "DataErrorHandling.h"
#include "EntityState.h"

namespace tpublic
{

	class EntityInstance
	{
	public:
		enum NetworkWriteFlag : uint8_t
		{
			NETWORK_WRITE_FLAG_ONLY_DIRTY = 0x01
		};

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
			int32_t					aTick)
		{
			m_state = aState;
			m_stateTick = aTick;
			m_dirty = true;
		}

		void
		AddComponent(
			ComponentBase*			aComponent)
		{
			// FIXME: this kinda defeats much of the purpose of using ECS. Will need some optimization at some point.
			m_components.push_back(std::unique_ptr<ComponentBase>(aComponent));
		}
		
		void
		WriteNetworkPublic(
			const ComponentManager* aComponentManager,
			IWriter*				aWriter,
			uint8_t					aFlags) const
		{
			bool onlyDirty = aFlags & NETWORK_WRITE_FLAG_ONLY_DIRTY;

			aWriter->WritePOD(m_state);

			uint32_t i = 0;
			for(const std::unique_ptr<ComponentBase>& component : m_components)
			{
				ComponentBase::Replication componentReplication = aComponentManager->GetComponentReplication(component->GetComponentId());

				if((component->IsDirty() || !onlyDirty) && componentReplication == ComponentBase::REPLICATION_PUBLIC)
				{
					aWriter->WriteUInt(i);
					aComponentManager->WriteNetwork(aWriter, component.get());
				}
				i++;
			}
		}

		void
		WriteNetworkPrivate(
			const ComponentManager* aComponentManager,
			IWriter*				aWriter,
			uint8_t					aFlags) const
		{
			bool onlyDirty = aFlags & NETWORK_WRITE_FLAG_ONLY_DIRTY;

			aWriter->WritePOD(m_state);

			uint32_t i = 0;
			for(const std::unique_ptr<ComponentBase>& component : m_components)
			{		
				ComponentBase::Replication componentReplication = aComponentManager->GetComponentReplication(component->GetComponentId());

				if((component->IsDirty() || !onlyDirty) && componentReplication == ComponentBase::REPLICATION_PRIVATE)
				{
					aWriter->WriteUInt(i);
					aComponentManager->WriteNetwork(aWriter, component.get());
				}
				i++;
			}
		}

		bool
		ReadNetwork(
			const ComponentManager*				aComponentManager,
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

				if(!aComponentManager->ReadNetwork(aReader, m_components[index].get()))
					return false;
			}
			return true;
		}

		bool
		IsDirty() const
		{
			if(m_dirty)
				return true;

			for (const std::unique_ptr<ComponentBase>& component : m_components)
			{
				if(component->IsDirty())
					return true;
			}
			return false;
		}

		void
		ResetDirty()
		{
			m_dirty = false;

			for (std::unique_ptr<ComponentBase>& component : m_components)
				component->ResetDirty();
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
		int32_t											GetStateTick() const { return m_stateTick; }
		
	private:
		
		uint32_t									m_entityId = 0;
		uint32_t									m_entityInstanceId = 0;
		EntityState::Id								m_state = EntityState::ID_DEFAULT;
		int32_t										m_stateTick = 0;
		std::vector<std::unique_ptr<ComponentBase>>	m_components;
		bool										m_dirty = false;
	};

}