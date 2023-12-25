#include "../Pcheader.h"

#include <tpublic/Components/Openable.h>

#include <tpublic/Systems/Openable.h>

namespace tpublic::Systems
{

	Openable::Openable(
		const Manifest*		aManifest)
		: SystemBase(aManifest, UPDATE_INTERVAL)
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
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			/*aContext*/) 
	{
		Components::Openable* openable = GetComponent<Components::Openable>(aComponents);

		if(openable->m_opened)
		{
			if(openable->m_despawn)
				return EntityState::ID_DESPAWNED;

			openable->m_opened = false;
		}

		return EntityState::CONTINUE;
	}

}