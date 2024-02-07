#pragma once

#include "Component.h"
#include "ComponentSchema.h"
#include "DataErrorHandling.h"
#include "Persistence.h"

namespace tpublic
{

	class ComponentBase
	{
	public:
		enum Flag : uint8_t
		{
			FLAG_PLAYER_ONLY				= 0x01,
			FLAG_REPLICATE_ONLY_ON_REQUEST	= 0x02
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

									ComponentBase();
		virtual						~ComponentBase();

		void						SetComponentId(
										uint32_t											aComponentId);
		void						ResetPendingPersistenceUpdate();
		void						SetPendingPersistenceUpdate(
										PendingPersistenceUpdate							aPendingPersistenceUpdate);
		void						ResetDirty();
		void						SetDirty();

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