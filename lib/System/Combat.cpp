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

	void		
	Combat::Update(
		ComponentBase**		aComponents) 
	{
		Components::Combat* combat = GetComponent<Components::Combat>(aComponents);

		bool isDead = combat->m_runtimeFlags & Components::Combat::RUNTIME_FLAG_DEAD;

		if(!isDead)
		{
			if(combat->m_currentHealth < combat->m_maxHealth)
			{
				combat->m_currentHealth++;
			}
		}
	}

}