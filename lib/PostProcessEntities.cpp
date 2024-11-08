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
			const Manifest*		aManifest,
			Data::Entity*		aEntity)
		{
			const NPCMetrics& npcMetrics = aManifest->m_npcMetrics;

			ApplyNPCMetrics::Process(
				&npcMetrics, 
				aEntity->m_modifiers, 
				aEntity->TryGetComponent<Components::CombatPublic>(),
				aEntity->TryGetComponent<Components::CombatPrivate>(),
				aEntity->TryGetComponent<Components::MinionPrivate>(),
				aEntity->TryGetComponent<Components::NPC>());
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