#include "Pcheader.h"

#include <tpublic/ComponentBase.h>
#include <tpublic/ComponentManager.h>
#include <tpublic/DataErrorHandling.h>
#include <tpublic/EntityInstance.h>

namespace tpublic
{

	EntityInstance::EntityInstance(
		uint32_t				aEntityId,
		uint32_t				aEntityInstanceId)
		: m_entityInstanceId(aEntityInstanceId)
		, m_entityId(aEntityId)
		, m_state(EntityState::INVALID_ID)
	{

	}

	EntityInstance::~EntityInstance()
	{
	}

	void
	EntityInstance::SetEntityInstanceId(
		uint32_t				aEntityInstanceId)
	{
		assert(m_entityInstanceId == 0);
		m_entityInstanceId = aEntityInstanceId;
	}

	void
	EntityInstance::SetState(
		EntityState::Id			aState,
		int32_t					aTick)
	{
		m_state = aState;
		m_stateTick = aTick;
		m_dirty = true;
	}

	void
	EntityInstance::AddComponent(
		ComponentBase*			aComponent)
	{
		// FIXME: this kinda defeats much of the purpose of using ECS. Will need some optimization at some point.
		m_components.push_back(std::unique_ptr<ComponentBase>(aComponent));
	}
		
	void
	EntityInstance::WriteNetworkPublic(
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
			uint8_t componentFlags = aComponentManager->GetComponentFlags(component->GetComponentId());

			if((component->IsDirty() || !onlyDirty) && componentReplication == ComponentBase::REPLICATION_PUBLIC && (componentFlags & ComponentBase::FLAG_REPLICATE_ONLY_ON_REQUEST) == 0)
			{
				aWriter->WriteUInt(i);
				aComponentManager->WriteNetwork(aWriter, component.get());
			}
			i++;
		}
	}

	void
	EntityInstance::WriteNetworkPrivate(
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
			uint8_t componentFlags = aComponentManager->GetComponentFlags(component->GetComponentId());

			if((component->IsDirty() || !onlyDirty) && componentReplication == ComponentBase::REPLICATION_PRIVATE && (componentFlags & ComponentBase::FLAG_REPLICATE_ONLY_ON_REQUEST) == 0)
			{
				aWriter->WriteUInt(i);
				aComponentManager->WriteNetwork(aWriter, component.get());
			}
			i++;
		}
	}

	bool
	EntityInstance::ReadNetwork(
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
	EntityInstance::IsDirty() const
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
	EntityInstance::ResetDirty()
	{
		m_dirty = false;

		for (std::unique_ptr<ComponentBase>& component : m_components)
			component->ResetDirty();
	}

	ComponentBase*
	EntityInstance::GetComponentBase(
		uint32_t							aComponentId)
	{
		for (std::unique_ptr<ComponentBase>& component : m_components)
		{
			if(component && component->GetComponentId() == aComponentId)
				return component.get();
		}
		return NULL;
	}

	const ComponentBase*
	EntityInstance::GetComponentBase(
		uint32_t							aComponentId) const
	{
		for (const std::unique_ptr<ComponentBase>& component : m_components)
		{
			if(component && component->GetComponentId() == aComponentId)
				return component.get();
		}
		return NULL;
	}

}