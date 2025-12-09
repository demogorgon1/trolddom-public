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
			Components::NPC*				aNPC,
			bool							aUpdate)
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
							Components::NPC::ResourceEntry* t = aNPC->m_resources.GetResourceEntry(resourceId);

							if(t == NULL)
							{	
								aNPC->m_resources.m_entries.resize(aNPC->m_resources.m_entries.size() + 1);
								t = &aNPC->m_resources.m_entries[aNPC->m_resources.m_entries.size() - 1];
							}
							else if(!aUpdate)
							{
								t = NULL;
							}

							if (t != NULL)
							{
								float value = (float)npcMetricsLevel->m_baseResource[resourceId] * modifier;

								if (aCombatPublic->IsElite())
									value *= npcMetricsLevel->m_eliteResource[resourceId];

								t->m_id = resourceId;
								t->m_max = (uint32_t)value;

								if (resourceId == Resource::ID_MANA)
								{
									// Initialize base mana 
									aCombatPrivate->m_baseMana = t->m_max;
								}
							}
						}
						else
						{
							// Minion
							Components::MinionPrivate::ResourceEntry* t = aMinionPrivate->m_resources.GetResourceEntry(resourceId);

							if (t == NULL)
							{
								aMinionPrivate->m_resources.m_entries.resize(aMinionPrivate->m_resources.m_entries.size() + 1);
								t = &aMinionPrivate->m_resources.m_entries[aMinionPrivate->m_resources.m_entries.size() - 1];
							}
							else if (!aUpdate)
							{
								t = NULL;
							}

							if (t != NULL)
							{
								float value = (float)npcMetricsLevel->m_baseResource[resourceId] * modifier;

								if (aCombatPublic->IsElite())
									value *= npcMetricsLevel->m_eliteResource[resourceId];

								t->m_id = resourceId;
								t->m_max = (uint32_t)value;

								if (resourceId == Resource::ID_MANA)
								{
									// Initialize base mana 
									aCombatPrivate->m_baseMana = t->m_max;
								}
							}
						}
					}

					// Do the same for weapon damage
					bool noWeaponDamageDefined = aCombatPrivate->m_weaponDamageRangeMin == 0 || aCombatPrivate->m_weaponDamageRangeMax == 0;
					if ((noWeaponDamageDefined || aUpdate) && aModifiers.m_weaponDamage.has_value())
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

		void		
		MakeEliteEasy(
			const NPCMetrics*			aNPCMetrics,
			Components::CombatPublic*	aCombatPublic,
			Components::CombatPrivate*	aCombatPrivate)
		{
			if(aCombatPublic != NULL && aCombatPrivate != NULL)
			{
				assert(aCombatPublic->IsElite());

				const NPCMetrics::Level* npcMetricsLevel = aNPCMetrics->GetLevel(aCombatPublic->m_level);

				if (npcMetricsLevel != NULL)
				{
					if(!aCombatPublic->m_easyElite)
					{
						Components::CombatPublic::ResourceEntry* health = aCombatPublic->GetResourceEntry(Resource::ID_HEALTH);

						if (health != NULL)
						{
							float eliteHealthMultiplier = npcMetricsLevel->m_eliteResource[Resource::ID_HEALTH];

							if (eliteHealthMultiplier > 1.0f)
							{
								float nonEliteHealth = (float)health->m_max / eliteHealthMultiplier;
								float updatedEliteHealthMultiplier = 1.0f + (eliteHealthMultiplier - 1.0f) / 6.5f;

								health->m_max = (uint32_t)(nonEliteHealth * updatedEliteHealthMultiplier);
								if (health->m_max == 0)
									health->m_max = 1;

								if (health->m_current > health->m_max)
									health->m_current = health->m_max;
							}
						}

						aCombatPublic->m_easyElite = true;
					}

					if(npcMetricsLevel->m_eliteWeaponDamage > 1.0f && !aCombatPrivate->m_easyElite)
					{
						float nonEliteWeaponDamageRangeMin = (float)aCombatPrivate->m_weaponDamageRangeMin / npcMetricsLevel->m_eliteWeaponDamage;
						float nonEliteWeaponDamageRangeMax = (float)aCombatPrivate->m_weaponDamageRangeMax / npcMetricsLevel->m_eliteWeaponDamage;

						float updatedEliteWeaponDamageMultiplier = 1.0f + (npcMetricsLevel->m_eliteWeaponDamage - 1.0f) / 4.0f;

						aCombatPrivate->m_weaponDamageRangeMin = (uint32_t)(nonEliteWeaponDamageRangeMin * updatedEliteWeaponDamageMultiplier);
						aCombatPrivate->m_weaponDamageRangeMax = (uint32_t)(nonEliteWeaponDamageRangeMax * updatedEliteWeaponDamageMultiplier);

						if(aCombatPrivate->m_weaponDamageRangeMin == 0)
							aCombatPrivate->m_weaponDamageRangeMin = 1;

						if (aCombatPrivate->m_weaponDamageRangeMax == 0)
							aCombatPrivate->m_weaponDamageRangeMax = 1;

						aCombatPrivate->m_easyElite = true;
					}
				}
			}
		}

	}

}