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

			// Default NPC stats
			ApplyNPCMetrics::Process(
				&npcMetrics, 
				aEntity->m_modifiers, 
				combatPublic,
				aEntity->TryGetComponent<Components::CombatPrivate>(),
				aEntity->TryGetComponent<Components::MinionPrivate>(),
				npc);
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