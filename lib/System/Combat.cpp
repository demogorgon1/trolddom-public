#include "../Pcheader.h"

#include <kaos-public/Components/Combat.h>

#include <kaos-public/Systems/Combat.h>

#include <kaos-public/EntityInstance.h>
#include <kaos-public/Resource.h>

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
			for(Components::Combat::Resource& resource : combat->m_resources)
			{
				switch(resource.m_id)
				{
				case Resource::ID_HEALTH:
					if(aEntityState != EntityState::ID_IN_COMBAT && resource.m_current < resource.m_max) 
						resource.m_current++;						
					break;

				case Resource::ID_MANA:
					if(resource.m_current < resource.m_max)
						resource.m_current++;
					break;

				case Resource::ID_RAGE:
					if (aEntityState != EntityState::ID_IN_COMBAT && resource.m_current > 0)
						resource.m_current--;
					break;

				case Resource::ID_ENERGY:
					if (resource.m_current < resource.m_max)
						resource.m_current++;
					break;

				default:
					break;
				}
			}
		}

		return EntityState::CONTINUE;
	}

}