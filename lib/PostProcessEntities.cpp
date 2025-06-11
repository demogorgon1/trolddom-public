#include "Pcheader.h"

#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/MinionPrivate.h>
#include <tpublic/Components/NPC.h>

#include <tpublic/Data/Entity.h>

#include <tpublic/ApplyNPCMetrics.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	namespace
	{

		void
		_PostProcessEntity(
			const Manifest*			aManifest,
			Data::Entity*			aEntity)
		{
			const NPCMetrics& npcMetrics = aManifest->m_npcMetrics;

			const Components::CombatPublic* combatPublic = aEntity->TryGetComponent<Components::CombatPublic>();
			Components::NPC* npc = aEntity->TryGetComponent<Components::NPC>();
			Components::CombatPrivate* combatPrivate = aEntity->TryGetComponent<Components::CombatPrivate>();

			// Default NPC stats
			ApplyNPCMetrics::Process(
				&npcMetrics, 
				aEntity->m_modifiers, 
				combatPublic,
				combatPrivate,
				aEntity->TryGetComponent<Components::MinionPrivate>(),
				npc);

			// Default melee push priorities
			if(npc != NULL && npc->m_meleePushPriority == 0)
			{				
				// Use weapon damage per second to calculate melee push priorty
				npc->m_meleePushPriority = ((combatPrivate->m_weaponDamageRangeMax - combatPrivate->m_weaponDamageRangeMin) * 5) / 20;
			}
		}

	}

	namespace PostProcessEntities
	{

		void		
		Run(
			Manifest* aManifest)
		{
			aManifest->GetContainer<Data::Entity>()->ForEach([aManifest](
				Data::Entity* aEntity)
			{
				_PostProcessEntity(aManifest, aEntity);
				return true;
			});
		}

	}

}