#include "../Pcheader.h"

#include <kaos-public/Components/Combat.h>

#include <kaos-public/Systems/Combat.h>

#include <kaos-public/EntityInstance.h>

namespace kaos_public::Systems
{

	Combat::Combat(
		const Manifest*		aManifest)
		: SystemBase(aManifest)
	{
		RequireComponent<Components::Combat>();
	}
	
	Combat::~Combat()
	{

	}

	//---------------------------------------------------------------------------

	EntityState::Id
	Combat::Update(
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
		ComponentBase**		aComponents,
		Context*			/*aContext*/) 
	{
		Components::Combat* combat = GetComponent<Components::Combat>(aComponents);

		if(aEntityState != EntityState::ID_DEAD)
		{
			if(combat->m_currentHealth < combat->m_maxHealth)
			{
				combat->m_currentHealth++;
			}
		}

		return EntityState::CONTINUE;
	}

}