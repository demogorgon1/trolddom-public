#pragma once

#include "ComponentBase.h"
#include "EntityState.h"

namespace tpublic
{

	class ComponentManager;

	class EntityInstance
	{
	public:
		enum NetworkWriteFlag : uint8_t
		{
			NETWORK_WRITE_FLAG_ONLY_DIRTY	= 0x01,
			NETWORK_WRITE_FLAG_PUBLIC		= 0x02,
			NETWORK_WRITE_FLAG_PRIVATE		= 0x04
		};

								EntityInstance(
									uint32_t							aEntityId,
									uint32_t							aEntityInstanceId);			
								~EntityInstance();

		void					SetEntityInstanceId(
									uint32_t							aEntityInstanceId);
		void					SetState(
									EntityState::Id						aState,
									int32_t								aTick);
		void					AddComponent(
									ComponentBase*						aComponent);
		void					WriteNetwork(
									const ComponentManager*				aComponentManager,
									IWriter*							aWriter,
									uint8_t								aFlags) const;
		bool					ReadNetwork(
									const ComponentManager*				aComponentManager,
									IReader*							aReader,
									std::vector<const ComponentBase*>*	aOutUpdatedComponents);
		bool					IsDirty() const;
		void					ResetDirty();
		ComponentBase*			GetComponentBase(
									uint32_t							aComponentId);
		const ComponentBase*	GetComponentBase(
									uint32_t							aComponentId) const;

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

		template <typename _T>
		bool
		HasComponent() const
		{
			for(const std::unique_ptr<ComponentBase>& component : m_components)
			{
				if(component && component->GetComponentId() == _T::ID)
					return true;
			}
			return false;
		}

		// Data access
		uint32_t										GetEntityInstanceId() const { assert(m_entityInstanceId != 0); return m_entityInstanceId; }
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