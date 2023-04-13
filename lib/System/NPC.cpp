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

	}
	
	NPC::~NPC()
	{

	}

	//--------------------------------------------------------------------------------------------

	void		
	NPC::Init(
		EntityInstance*			aEntity) 
	{
		Components::Combat* combat = aEntity->GetComponent<Components::Combat>();
		Components::NPC* npc = aEntity->GetComponent<Components::NPC>();

		combat->m_currentHealth = npc->m_stats.m_stats[Stat::ID_HEALTH];
		combat->m_maxHealth = npc->m_stats.m_stats[Stat::ID_HEALTH];
	}

	void		
	NPC::Update(
		EntityInstance*			aEntity) 
	{
		Components::Combat* combat = aEntity->GetComponent<Components::Combat>();
		Components::NPC* npc = aEntity->GetComponent<Components::NPC>();
		Components::Position* position = aEntity->GetComponent<Components::Position>();

		bool isDead = combat->m_runtimeFlags & Components::Combat::RUNTIME_FLAG_DEAD;

		(void)npc;
		(void)position;
		(void)isDead;
	}

}

