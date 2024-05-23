#include "../Pcheader.h"

#include <tpublic/Components/Trigger.h>

#include <tpublic/Systems/Trigger.h>

namespace tpublic::Systems
{

	Trigger::Trigger(
		const SystemData* aData)
		: SystemBase(aData, UPDATE_INTERVAL)
	{
		RequireComponent<Components::Trigger>();
	}
	
	Trigger::~Trigger()
	{

	}

	//-----------------------------------------------------------------------------

	EntityState::Id	
	Trigger::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			aContext) 
	{
		Components::Trigger* trigger = GetComponent<Components::Trigger>(aComponents);
		
		// Remove expired lock outs
		for(Components::Trigger::LockOutTable::iterator i = trigger->m_lockOutTable.begin(); i != trigger->m_lockOutTable.end();)
		{
			if(i->second > aContext->m_tick)
				trigger->m_lockOutTable.erase(i++);
			else
				i++;
		}

		return EntityState::CONTINUE;
	}
	
}
