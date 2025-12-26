#include "../Pcheader.h"

#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/NPC.h>
#include <tpublic/Components/PlayerPublic.h>
#include <tpublic/Components/Tag.h>

#include <tpublic/DirectEffects/StartRoute.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IResourceChangeQueue.h>

namespace tpublic::DirectEffects
{

	void
	StartRoute::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if (aChild->m_name == "route")
					m_routeId = aChild->GetId(DataType::ID_ROUTE);
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
			}
		});
	}

	void
	StartRoute::ToStream(
		IWriter*						aStream) const 
	{	
		ToStreamBase(aStream);

		aStream->WriteUInt(m_routeId);
	}

	bool
	StartRoute::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		
		if(!aStream->ReadUInt(m_routeId))
			return false;
		return true;
	}

	DirectEffectBase::Result
	StartRoute::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					/*aRandom*/,
		const Manifest*					/*aManifest*/,
		CombatEvent::Id					/*aId*/,
		uint32_t						/*aAbilityId*/,
		const SourceEntityInstance&		/*aSourceEntityInstance*/,
		EntityInstance*					aSource,
		EntityInstance*					aTarget,
		const Vec2&						/*aAOETarget*/,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			aCombatResultQueue,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					/*aEventQueue*/,
		const IWorldView*				/*aWorldView*/,
		bool							/*aOffHand*/)
	{				
		if(aSource != NULL && aTarget != NULL)
		{
			if(aTarget->IsPlayer())
			{
				// Escort quest
				Components::NPC* npc = aSource->GetComponent<Components::NPC>();
				Components::Tag* tag = aSource->GetComponent<Components::Tag>();

				if (npc != NULL && tag != NULL)
				{
					if (!tag->m_playerTag.IsSet() && npc->m_routeId == 0)
					{
						Components::CombatPublic* targetCombatPublic = aTarget->GetComponent<Components::CombatPublic>();
						Components::PlayerPublic* targetPlayerPublic = aTarget->GetComponent<Components::PlayerPublic>();
						assert(targetCombatPublic != NULL && targetPlayerPublic != NULL);

						// Need to tag the NPC so the player/group can get credit if required
						PlayerTag playerTag;
						if (targetCombatPublic->m_combatGroupId != 0)
							playerTag.SetGroupId(targetCombatPublic->m_combatGroupId);
						else
							playerTag.SetCharacter(targetPlayerPublic->m_characterId, targetCombatPublic->m_level);

						uint32_t routeId = m_routeId;
						uint32_t playerEntityInstanceId = aTarget->GetEntityInstanceId();

						aCombatResultQueue->AddUpdateCallback([npc, tag, routeId, playerTag, playerEntityInstanceId]()
						{
							npc->m_routeId = routeId;
							npc->m_routeIsReversing = false;
							npc->m_subRouteIndex = 0;
							npc->m_effectiveRouteId = routeId;

							if (playerTag.IsSet())
							{
								tag->m_playerEntityInstanceId = playerEntityInstanceId;
								tag->m_playerTag = playerTag;
								tag->m_lootRule = tpublic::LootRule::INVALID_ID;
								tag->m_lootThreshold = tpublic::Rarity::INVALID_ID;
								tag->SetDirty();
							}
						});
					}
				}
			}
			else
			{
				Components::NPC* npc = aTarget->GetComponent<Components::NPC>();

				if(npc != NULL)
				{
					// Not escort, just plain routing
					uint32_t routeId = m_routeId;

					aCombatResultQueue->AddUpdateCallback([npc, routeId]()
					{
						npc->m_routeId = routeId;
						npc->m_routeIsReversing = false;
						npc->m_subRouteIndex = 0;
						npc->m_effectiveRouteId = routeId;
					});
				}
			}
		}

		return Result();
	}

}