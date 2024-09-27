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

		struct ComponentEntry
		{
			ComponentBase*			m_allocated = NULL;
			const ComponentBase*	m_static = NULL;
		};

								EntityInstance(
									ComponentManager*					aComponentManager,
									uint32_t							aEntityId,
									uint32_t							aEntityInstanceId);			
								~EntityInstance();

		void					SetEntityInstanceId(
									uint32_t							aEntityInstanceId);
		void					SetState(
									EntityState::Id						aState,
									int32_t								aTick);
		void					AddComponent(
									ComponentEntry&						aComponentEntry);
		void					WriteNetwork(
									IWriter*							aWriter,
									uint8_t								aFlags) const;
		bool					ReadNetwork(
									IReader*							aReader,
									std::vector<const ComponentBase*>*	aOutUpdatedComponents);
		bool					IsDirty() const;
		void					ResetDirty();
		ComponentBase*			GetComponentBase(
									uint32_t							aComponentId);
		const ComponentBase*	GetComponentBase(
									uint32_t							aComponentId) const;
		void					IncrementSeq();

		template <typename _T>
		_T*
		GetComponent()
		{
			for(ComponentEntry& component : m_components)
			{
				if (component.m_allocated != NULL && component.m_allocated->GetComponentId() == _T::ID)
					return (_T*)component.m_allocated;
				else if (component.m_static != NULL && component.m_static->GetComponentId() == _T::ID)
					return (_T*)component.m_static;
			}
			return NULL;
		}

		template <typename _T>
		const _T*
		GetComponent() const
		{
			for(const ComponentEntry& component : m_components)
			{
				if (component.m_allocated != NULL && component.m_allocated->GetComponentId() == _T::ID)
					return (const _T*)component.m_allocated;
				else if (component.m_static != NULL && component.m_static->GetComponentId() == _T::ID)
					return (const _T*)component.m_static;
			}
			return NULL;
		}

		template <typename _T>
		bool
		HasComponent() const
		{
			for(const ComponentEntry& component : m_components)
			{
				if (component.m_allocated != NULL && component.m_allocated->GetComponentId() == _T::ID)
					return true;
				else if (component.m_static != NULL && component.m_static->GetComponentId() == _T::ID)
					return true;
			}
			return false;
		}

		// Data access
		uint32_t										GetEntityInstanceId() const { assert(m_entityInstanceId != 0); return m_entityInstanceId; }
		uint32_t										GetEntityId() const { return m_entityId; }
		EntityState::Id									GetState() const { return m_state; }
		bool											IsPlayer() const { return m_entityId == 0; }
		std::vector<ComponentEntry>&					GetComponents() { return m_components; }
		int32_t											GetStateTick() const { return m_stateTick; }
		uint8_t											GetSeq() const { return m_seq; }
		
	private:
		
		ComponentManager*							m_componentManager;
		uint32_t									m_entityId = 0;
		uint32_t									m_entityInstanceId = 0;
		EntityState::Id								m_state = EntityState::ID_DEFAULT;
		int32_t										m_stateTick = 0;
		uint8_t										m_seq = 0;
		std::vector<ComponentEntry>					m_components;
		bool										m_dirty = false;
	};

}