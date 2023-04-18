#include "../Pcheader.h"

#include <kpublic/Components/CombatPublic.h>

#include <kpublic/Systems/Combat.h>

#include <kpublic/EntityInstance.h>
#include <kpublic/Resource.h>

namespace kpublic::Systems
{

	Combat::Combat(
		const Manifest*		aManifest)
		: SystemBase(aManifest)
	{
		RequireComponent<Components::CombatPublic>();
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
		Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);

		if(aEntityState != EntityState::ID_DEAD)
		{
			for(Components::CombatPublic::Resource& resource : combatPublic->m_resources)
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