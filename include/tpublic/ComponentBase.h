#pragma once

#include "Component.h"
#include "ComponentSchema.h"
#include "DataErrorHandling.h"
#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"
#include "Persistence.h"

namespace tpublic
{

	class ComponentBase
	{
	public:
		enum Flag : uint8_t
		{
			FLAG_PLAYER_ONLY			= 0x01
		};

		enum Replication : uint8_t
		{
			REPLICATION_NONE,
			REPLICATION_PUBLIC,
			REPLICATION_PRIVATE
		};

		enum PendingPersistenceUpdate : uint8_t
		{
			PENDING_PERSISTENCE_UPDATE_NONE,
			PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY,
			PENDING_PERSISTENCE_UPDATE_MEDIUM_PRIORITY,
			PENDING_PERSISTENCE_UPDATE_HIGH_PRIORITY
		};

		ComponentBase()
		{

		}

		virtual 
		~ComponentBase()
		{
			// FIXME: For now we'll need a virtual destructor, but with a proper component pool we can get rid of it (and the vtable)
		}

		template <typename _T>
		const _T*
		Cast() const
		{
			TP_CHECK(m_componentId == _T::ID, "Component type mismatch.");
			return (const _T*)this;
		}

		template <typename _T>
		_T*
		Cast()
		{
			TP_CHECK(m_componentId == _T::ID, "Component type mismatch.");
			return (_T*)this;
		}

		template <typename _T>
		bool
		Is() const
		{
			return m_componentId == _T::ID;
		}

		void
		SetComponentId(
			uint32_t											aComponentId)
		{
			assert(m_componentId == 0);
			m_componentId = aComponentId;
		}

		void
		ResetPendingPersistenceUpdate()
		{
			m_pendingPersistenceUpdate = PENDING_PERSISTENCE_UPDATE_NONE;
		}

		void
		SetPendingPersistenceUpdate(
			PendingPersistenceUpdate							aPendingPersistenceUpdate)
		{
			if((uint8_t)aPendingPersistenceUpdate > (uint8_t)m_pendingPersistenceUpdate)
				m_pendingPersistenceUpdate = aPendingPersistenceUpdate;

			m_dirty = true;
		}

		void
		ResetDirty()
		{
			m_dirty = false;
		}

		void
		SetDirty()
		{
			m_dirty = true;
		}

		// Data access
		uint32_t					GetComponentId() const { return m_componentId; }
		PendingPersistenceUpdate	GetPendingPersistenceUpdate() const { return m_pendingPersistenceUpdate; }
		bool						IsDirty() const { return m_dirty; }

	private:

		uint32_t					m_componentId = 0;
		PendingPersistenceUpdate	m_pendingPersistenceUpdate = PENDING_PERSISTENCE_UPDATE_NONE;
		bool						m_dirty = false;
	};

}