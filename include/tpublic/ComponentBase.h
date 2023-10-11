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

	class Manifest;

	class ComponentBase
	{
	public:
		enum Flag : uint8_t
		{
			FLAG_PUBLIC					= 0x01,
			FLAG_REPLICATE_TO_OWNER		= 0x02,
			FLAG_REPLICATE_TO_OTHERS	= 0x04,
			FLAG_PLAYER_ONLY			= 0x08
		};

		enum PendingPersistenceUpdate : uint8_t
		{
			PENDING_PERSISTENCE_UPDATE_NONE,
			PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY,
			PENDING_PERSISTENCE_UPDATE_MEDIUM_PRIORITY,
			PENDING_PERSISTENCE_UPDATE_HIGH_PRIORITY
		};

		ComponentBase(
			uint32_t		aComponentId,
			uint8_t			aFlags,
			Persistence::Id	aPersistence)
			: m_componentId(aComponentId)
			, m_flags(aFlags)
			, m_persistence(aPersistence)
			, m_pendingPersistenceUpdate(PENDING_PERSISTENCE_UPDATE_NONE)
		{

		}

		virtual 
		~ComponentBase()
		{

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
		}

		// Data access
		uint32_t					GetComponentId() const { return m_componentId; }
		uint8_t						GetFlags() const { return m_flags; }
		Persistence::Id				GetPersistence() const { return m_persistence; }
		PendingPersistenceUpdate	GetPendingPersistenceUpdate() const { return m_pendingPersistenceUpdate; }

	private:

		uint32_t					m_componentId;
		uint8_t						m_flags;
		Persistence::Id				m_persistence;
		PendingPersistenceUpdate	m_pendingPersistenceUpdate;
	};

}