#include "Pcheader.h"

#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/NPC.h>

#include <tpublic/Data/Entity.h>

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

			const Components::CombatPublic* combatPublic = aEntity->TryGetComponent<Components::CombatPublic>();
			Components::CombatPrivate* combatPrivate = aEntity->TryGetComponent<Components::CombatPrivate>();
			Components::NPC* npc = aEntity->TryGetComponent<Components::NPC>();

			if(combatPublic != NULL && combatPrivate != NULL && npc != NULL)
			{
				const NPCMetrics::Level* npcMetricsLevel = npcMetrics.GetLevel(combatPublic->m_level);

				if(npcMetricsLevel != NULL)
				{					
					// Fill out missing resources based on NPC metrics per level and modifiers for this specific entity type
					for(Data::Entity::Modifiers::Resources::const_iterator i = aEntity->m_modifiers.m_resources.cbegin(); i != aEntity->m_modifiers.m_resources.cend(); i++)
					{
						Resource::Id resourceId = i->first;
						float modifier = i->second;

						if(npc->m_resources.GetResourceEntry(resourceId) == NULL)
						{
							float value = (float)npcMetricsLevel->m_baseResource[resourceId] * modifier;						

							if(combatPublic->IsElite())
								value *= npcMetricsLevel->m_eliteResource[resourceId];

							Components::NPC::ResourceEntry resourceEntry;
							resourceEntry.m_id = resourceId;
							resourceEntry.m_max = (uint32_t)value;
							npc->m_resources.m_entries.push_back(resourceEntry);							
						}
					}

					// Do the same for weapon damage
					bool noWeaponDamageDefined = combatPrivate->m_weaponDamageRangeMin == 0 || combatPrivate->m_weaponDamageRangeMax == 0;
					if(noWeaponDamageDefined && aEntity->m_modifiers.m_weaponDamage.has_value())
					{
						float factor = aEntity->m_modifiers.m_weaponDamage.value();

						if (combatPublic->IsElite())
							factor *= npcMetricsLevel->m_eliteWeaponDamage;

						combatPrivate->m_weaponDamageRangeMin = (uint32_t)(factor * (float)npcMetricsLevel->m_baseWeaponDamage.m_min);
						combatPrivate->m_weaponDamageRangeMax = (uint32_t)(factor * (float)npcMetricsLevel->m_baseWeaponDamage.m_max);
					}
				}
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