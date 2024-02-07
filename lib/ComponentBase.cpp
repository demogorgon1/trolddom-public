#include "Pcheader.h"

#include <tpublic/ComponentBase.h>

namespace tpublic
{

	ComponentBase::ComponentBase()
	{

	}

	ComponentBase::~ComponentBase()
	{
		// FIXME: For now we'll need a virtual destructor, but with a proper component pool we can get rid of it (and the vtable)
	}

	void
	ComponentBase::SetComponentId(
		uint32_t											aComponentId)
	{
		assert(m_componentId == 0);
		m_componentId = aComponentId;
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