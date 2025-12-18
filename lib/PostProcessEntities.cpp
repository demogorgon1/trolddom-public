#include "Pcheader.h"

#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/MinionPrivate.h>
#include <tpublic/Components/NPC.h>
#include <tpublic/Components/Sprite.h>

#include <tpublic/Data/Entity.h>
#include <tpublic/Data/Sprite.h>

#include <tpublic/ApplyNPCMetrics.h>
#include <tpublic/Manifest.h>
#include <tpublic/NPCMetrics.h>

namespace tpublic
{

	namespace
	{

		void
		_PostProcessEntity(
			const Manifest*			aManifest,
			Data::Entity*			aEntity)
		{
			const NPCMetrics& npcMetrics = *aManifest->m_npcMetrics;

			Components::CombatPublic* combatPublic = aEntity->TryGetComponent<Components::CombatPublic>();
			Components::NPC* npc = aEntity->TryGetComponent<Components::NPC>();
			Components::CombatPrivate* combatPrivate = aEntity->TryGetComponent<Components::CombatPrivate>();

			// Default NPC stats
			ApplyNPCMetrics::Process(
				&npcMetrics, 
				aEntity->m_modifiers, 
				combatPublic,
				combatPrivate,
				aEntity->TryGetComponent<Components::MinionPrivate>(),
				npc,
				false);

			// Default melee push priorities
			if(npc != NULL && npc->m_meleePushPriority == 0)
			{				
				// Use weapon damage per second to calculate melee push priorty
				npc->m_meleePushPriority = ((combatPrivate->m_weaponDamageRangeMax - combatPrivate->m_weaponDamageRangeMin) * 5) / 20;
			}

			// Identify NPCs with head anchor
			if(npc != NULL)
			{
				const Components::Sprite* sprite = aEntity->TryGetComponent<Components::Sprite>();
				if(sprite != NULL)
				{
					for(const std::unique_ptr<Components::Sprite::Animation>& animation : sprite->m_animations)
					{
						for(uint32_t frameSpriteId : animation->m_frameSpriteIds)
						{
							const Data::Sprite* spriteData = aManifest->GetById<Data::Sprite>(frameSpriteId);
							if(spriteData->m_info.m_headAnchor.has_value())
							{
								npc->m_hasHeadAnchor = true;
								break;
							}
						}

						if(npc->m_hasHeadAnchor)
							break;
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