#include "../Pcheader.h"

#include <kaos-public/Components/Combat.h>
#include <kaos-public/Components/NPC.h>
#include <kaos-public/Components/Position.h>

#include <kaos-public/Systems/NPC.h>

#include <kaos-public/EntityInstance.h>

namespace kaos_public::Systems
{

	NPC::NPC(
		const Manifest*			aManifest)
		: SystemBase(aManifest)
	{
		RequireComponent<Components::Combat>();
		RequireComponent<Components::NPC>();
		RequireComponent<Components::Position>();
	}
	
	NPC::~NPC()
	{

	}

	//--------------------------------------------------------------------------------------------

	void		
	NPC::Init(
		ComponentBase**			aComponents) 
	{
		Components::Combat* combat = GetComponent<Components::Combat>(aComponents);
		Components::NPC* npc = GetComponent<Components::NPC>(aComponents);

		combat->m_currentHealth = npc->m_stats.m_stats[Stat::ID_HEALTH];
		combat->m_maxHealth = npc->m_stats.m_stats[Stat::ID_HEALTH];
	}

	void		
	NPC::Update(
		ComponentBase**			aComponents) 
	{
		Components::Combat* combat = GetComponent<Components::Combat>(aComponents);
		Components::NPC* npc = GetComponent<Components::NPC>(aComponents);
		Components::Position* position = GetComponent<Components::Position>(aComponents);

		bool isDead = combat->m_runtimeFlags & Components::Combat::RUNTIME_FLAG_DEAD;

		(void)npc;
		(void)position;
		(void)isDead;
	}

}

