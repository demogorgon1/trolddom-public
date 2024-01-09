#include "../Pcheader.h"

#include <tpublic/Components/Openable.h>

#include <tpublic/Systems/Openable.h>

namespace tpublic::Systems
{

	Openable::Openable(
		const SystemData*	aData)
		: SystemBase(aData, UPDATE_INTERVAL)
	{
		RequireComponent<Components::Openable>();
	}
	
	Openable::~Openable()
	{

	}

	//---------------------------------------------------------------------------

	EntityState::Id
	Openable::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		int32_t				aTicksInState,
		ComponentBase**		aComponents,
		Context*			/*aContext*/) 
	{
		Components::Openable* openable = GetComponent<Components::Openable>(aComponents);

		if(openable->m_duration != 0 && aTicksInState >= (int32_t)openable->m_duration)
			return EntityState::ID_DESPAWNED;

		if(openable->m_opened)
		{
			if(openable->m_despawn)
				return EntityState::ID_DESPAWNED;

			openable->m_opened = false;
		}

		return EntityState::CONTINUE;
	}

}