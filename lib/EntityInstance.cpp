#include "Pcheader.h"

#include <tpublic/ComponentBase.h>
#include <tpublic/ComponentManager.h>
#include <tpublic/DataErrorHandling.h>
#include <tpublic/EntityInstance.h>

namespace tpublic
{

	EntityInstance::EntityInstance(
		ComponentManager*		aComponentManager,
		uint32_t				aEntityId,
		uint32_t				aEntityInstanceId)
		: m_entityInstanceId(aEntityInstanceId)
		, m_entityId(aEntityId)
		, m_state(EntityState::INVALID_ID)
		, m_componentManager(aComponentManager)
	{

	}

	EntityInstance::~EntityInstance()
	{
		for (ComponentEntry& t : m_components)
		{
			if(t.m_allocated != NULL)
				m_componentManager->ReleaseComponent(t.m_allocated);
		}
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
		ComponentEntry&			aComponentEntry)
	{
		m_components.push_back(aComponentEntry);
	}
		
	void
	EntityInstance::WriteNetwork(
		IWriter*				aWriter,
		uint8_t					aFlags) const
	{
		bool onlyDirtyFlag = aFlags & NETWORK_WRITE_FLAG_ONLY_DIRTY;
		bool publicFlag = aFlags & NETWORK_WRITE_FLAG_PUBLIC;
		bool privateFlag = aFlags & NETWORK_WRITE_FLAG_PRIVATE;

		aWriter->WritePOD(m_state);

		uint32_t i = 0;
		for(const ComponentEntry& component : m_components)
		{
			if(component.m_allocated != NULL)
			{
				ComponentBase::Replication componentReplication = m_componentManager->GetComponentReplication(component.m_allocated->GetComponentId());
				uint8_t componentFlags = m_componentManager->GetComponentFlags(component.m_allocated->GetComponentId());

				if ((component.m_allocated->IsDirty() || !onlyDirtyFlag) &&
					(componentFlags & ComponentBase::FLAG_REPLICATE_ONLY_ON_REQUEST) == 0 &&
					((componentReplication == ComponentBase::REPLICATION_PUBLIC && publicFlag) || (componentReplication == ComponentBase::REPLICATION_PRIVATE && privateFlag)))
				{
					aWriter->WriteUInt(i);
					m_componentManager->WriteNetwork(aWriter, component.m_allocated);
				}
			}

			i++;
		}
	}

	bool
	EntityInstance::ReadNetwork(
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

			ComponentEntry& component = m_components[index];

			if(component.m_allocated == NULL)
				return false;

			if (aOutUpdatedComponents != NULL)
				aOutUpdatedComponents->push_back(component.m_allocated);

			if(!m_componentManager->ReadNetwork(aReader, component.m_allocated))
				return false;
		}
		return true;
	}

	bool
	EntityInstance::IsDirty() const
	{
		if(m_dirty)
			return true;

		for (const ComponentEntry& component : m_components)
		{
			if(component.m_allocated != NULL && component.m_allocated->IsDirty())
				return true;
		}
		return false;
	}

	void
	EntityInstance::ResetDirty()
	{
		m_dirty = false;

		for (ComponentEntry& component : m_components)
		{
			if(component.m_allocated != NULL)
				component.m_allocated->ResetDirty();
		}
	}

	ComponentBase*
	EntityInstance::GetComponentBase(
		uint32_t							aComponentId)
	{
		for (ComponentEntry& component : m_components)
		{
			if(component.m_allocated != NULL && component.m_allocated->GetComponentId() == aComponentId)
			{
				return component.m_allocated;
			}
			else if (component.m_static != NULL && component.m_static->GetComponentId() == aComponentId)
			{
				// FIXME: shouldn't do this. This essentially allows systems to update static components. 
				return (ComponentBase*)component.m_static;
			}			
		}
		return NULL;
	}

	const ComponentBase*
	EntityInstance::GetComponentBase(
		uint32_t							aComponentId) const
	{
		for (const ComponentEntry& component : m_components)
		{
			if (component.m_allocated != NULL && component.m_allocated->GetComponentId() == aComponentId)
				return component.m_allocated;
			else if (component.m_static != NULL && component.m_static->GetComponentId() == aComponentId)
				return component.m_static;
		}
		return NULL;
	}

}