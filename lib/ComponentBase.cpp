#include "Pcheader.h"

#include <tpublic/ComponentBase.h>

namespace tpublic
{

	void
	ComponentBase::SetComponentId(
		uint32_t											aComponentId)
	{
		assert(m_componentId == 0);
		m_componentId = aComponentId;
	}

	void						
	ComponentBase::InitAllocation(
		uint32_t											aComponentId,
		ComponentPoolChunkBase*								aComponentPoolChunkBase,
		uint8_t												aIndex)
	{
		m_componentId = aComponentId;
		m_componentPoolChunkBase = aComponentPoolChunkBase;
		m_componentPoolChunkIndex = aIndex;
	}

	void
	ComponentBase::ResetPendingPersistenceUpdate()
	{
		m_pendingPersistenceUpdate = PENDING_PERSISTENCE_UPDATE_NONE;
	}

	void
	ComponentBase::SetPendingPersistenceUpdate(
		PendingPersistenceUpdate							aPendingPersistenceUpdate)
	{
		if((uint8_t)aPendingPersistenceUpdate > (uint8_t)m_pendingPersistenceUpdate)
			m_pendingPersistenceUpdate = aPendingPersistenceUpdate;

		m_dirty = true;
	}

	void
	ComponentBase::ResetDirty()
	{
		m_dirty = false;
	}

	void
	ComponentBase::SetDirty()
	{
		m_dirty = true;
	}

}