#include "Pcheader.h"

#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/MinionPrivate.h>
#include <tpublic/Components/NPC.h>

#include <tpublic/ApplyNPCMetrics.h>
#include <tpublic/NPCMetrics.h>

namespace tpublic
{

	namespace ApplyNPCMetrics
	{

		void		
		Process(
			const NPCMetrics*				aNPCMetrics,
			const Data::Entity::Modifiers&	aModifiers,
			const Components::CombatPublic*	aCombatPublic,
			Components::CombatPrivate*		aCombatPrivate,
			Components::MinionPrivate*		aMinionPrivate,
			Components::NPC*				aNPC)
		{
			if (aCombatPublic != NULL && aCombatPrivate != NULL && (aNPC != NULL || aMinionPrivate != NULL))
			{
				const NPCMetrics::Level* npcMetricsLevel = aNPCMetrics->GetLevel(aCombatPublic->m_level);

				if (npcMetricsLevel != NULL)
				{
					// Fill out missing resources based on NPC metrics per level and modifiers for this specific entity type
					for (Data::Entity::Modifiers::Resources::const_iterator i = aModifiers.m_resources.cbegin(); i != aModifiers.m_resources.cend(); i++)
					{
						Resource::Id resourceId = i->first;
						float modifier = i->second;

						if (aNPC != NULL)
						{
							// NPC
							if (aNPC->m_resources.GetResourceEntry(resourceId) == NULL)
							{
								float value = (float)npcMetricsLevel->m_baseResource[resourceId] * modifier;

								if (aCombatPublic->IsElite())
									value *= npcMetricsLevel->m_eliteResource[resourceId];

								Components::NPC::ResourceEntry resourceEntry;
								resourceEntry.m_id = resourceId;
								resourceEntry.m_max = (uint32_t)value;
								aNPC->m_resources.m_entries.push_back(resourceEntry);

								if (resourceId == Resource::ID_MANA)
								{
									// Initialize base mana 
									aCombatPrivate->m_baseMana = resourceEntry.m_max;
								}
							}
						}
						else
						{
							// Minion
							if (aMinionPrivate->m_resources.GetResourceEntry(resourceId) == NULL)
							{
								float value = (float)npcMetricsLevel->m_baseResource[resourceId] * modifier;

								if (aCombatPublic->IsElite())
									value *= npcMetricsLevel->m_eliteResource[resourceId];

								Components::MinionPrivate::ResourceEntry resourceEntry;
								resourceEntry.m_id = resourceId;
								resourceEntry.m_max = (uint32_t)value;
								aMinionPrivate->m_resources.m_entries.push_back(resourceEntry);

								if (resourceId == Resource::ID_MANA)
								{
									// Initialize base mana 
									aCombatPrivate->m_baseMana = resourceEntry.m_max;
								}
							}
						}
					}

					// Do the same for weapon damage
					bool noWeaponDamageDefined = aCombatPrivate->m_weaponDamageRangeMin == 0 || aCombatPrivate->m_weaponDamageRangeMax == 0;
					if (noWeaponDamageDefined && aModifiers.m_weaponDamage.has_value())
					{
						float factor = aModifiers.m_weaponDamage.value();

						if (aCombatPublic->IsElite())
							factor *= npcMetricsLevel->m_eliteWeaponDamage;

						aCombatPrivate->m_weaponDamageRangeMin = (uint32_t)(factor * (float)npcMetricsLevel->m_baseWeaponDamage.m_min);
						aCombatPrivate->m_weaponDamageRangeMax = (uint32_t)(factor * (float)npcMetricsLevel->m_baseWeaponDamage.m_max);
					}

					// Set armor
					aCombatPrivate->m_armor = npcMetricsLevel->m_baseArmor;

					if (aModifiers.m_armor.has_value())
					{
						float factor = aModifiers.m_armor.value();

						aCombatPrivate->m_armor = (uint32_t)(factor * (float)aCombatPrivate->m_armor);
					}
				}
			}
		}

	}

}