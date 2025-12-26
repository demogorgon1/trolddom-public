#include "../Pcheader.h"

#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/Position.h>

#include <tpublic/Data/Ability.h>

#include <tpublic/DirectEffects/TriggerAbility.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Manifest.h>

namespace tpublic::DirectEffects
{

	void
	TriggerAbility::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if (aChild->m_name == "ability")
					m_abilityId = aChild->GetId(DataType::ID_ABILITY);
				else if(aChild->m_name == "target")
					m_target = SourceToTarget(aChild);
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
			}
		});
	}

	void
	TriggerAbility::ToStream(
		IWriter*						aStream) const 
	{	
		ToStreamBase(aStream);

		aStream->WriteUInt(m_abilityId);
		aStream->WritePOD(m_target);
	}

	bool
	TriggerAbility::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		
		if(!aStream->ReadUInt(m_abilityId))
			return false;
		if(!aStream->ReadPOD(m_target))
			return false;
		return true;
	}

	DirectEffectBase::Result
	TriggerAbility::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					aRandom,
		const Manifest*					aManifest,
		CombatEvent::Id					/*aId*/,
		uint32_t						/*aAbilityId*/,
		const SourceEntityInstance&		aSourceEntityInstance,
		EntityInstance*					aSource,
		EntityInstance*					aTarget,
		const Vec2&						/*aAOETarget*/,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			/*aCombatResultQueue*/,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					aEventQueue,
		const IWorldView*				aWorldView,
		bool							/*aOffHand*/)
	{				
		const Data::Ability* ability = aManifest->GetById<Data::Ability>(m_abilityId);
		const Components::Position* targetPosition = aTarget->GetComponent<Components::Position>();
		const Components::CombatPublic* targetCombatPublic = aTarget->GetComponent<Components::CombatPublic>();

		if(targetCombatPublic == NULL)
			return Result();

		std::vector<const EntityInstance*> possibleTargets;

		switch(m_target)
		{
		case TARGET_BEHIND:
			{
				const Components::Position* sourcePosition = aSource->GetComponent<Components::Position>();
				Vec2 behindPosition{
					sourcePosition->m_position.m_x + (targetPosition->m_position.m_x - sourcePosition->m_position.m_x) * 2,
					sourcePosition->m_position.m_y + (targetPosition->m_position.m_y - sourcePosition->m_position.m_y) * 2 };

				IWorldView::EntityQuery query;
				query.m_flags = 0;
				query.m_maxDistance = 0;
				query.m_position = behindPosition;

				aWorldView->WorldViewQueryEntityInstances(query, [&](
					const EntityInstance*	aEntityInstance,
					int32_t					/*aDistanceSquared*/)
				{
					if(aEntityInstance != aTarget && EntityState::CanBeAttacked(aEntityInstance->GetState()))
					{
						const Components::CombatPublic* combatPublic = aEntityInstance->GetComponent<Components::CombatPublic>();
						if(combatPublic != NULL && combatPublic->m_factionId == targetCombatPublic->m_factionId)
						{
							possibleTargets.push_back(aEntityInstance);
							return true;
						}
					}

					return false; // Continue
				});
			}
			break;

		case TARGET_RANDOM_NEARBY:
			{
				IWorldView::EntityQuery query;
				query.m_flags = IWorldView::EntityQuery::FLAG_LINE_OF_SIGHT;
				query.m_maxDistance = ability->m_range;
				query.m_position = targetPosition->m_position;

				aWorldView->WorldViewQueryEntityInstances(query, [&](
					const EntityInstance*	aEntityInstance,
					int32_t					/*aDistanceSquared*/)
				{
					if(aEntityInstance != aTarget && EntityState::CanBeAttacked(aEntityInstance->GetState()))
					{
						// FIXME: for now we just assume the target must be the same faction as the original target - works fine for chain-type effects
						const Components::CombatPublic* combatPublic = aEntityInstance->GetComponent<Components::CombatPublic>();
						if(combatPublic != NULL && combatPublic->m_factionId == targetCombatPublic->m_factionId)
							possibleTargets.push_back(aEntityInstance);
					}

					return false; // Continue
				});
			}
			break;

		default:
			break;
		}

		if(possibleTargets.size() > 0)
		{
			const EntityInstance* selectedTarget = Helpers::RandomItem(aRandom, possibleTargets);

			aEventQueue->EventQueueAbility(aSourceEntityInstance, selectedTarget->GetEntityInstanceId(), Vec2(), ability, ItemInstanceReference(), NULL, targetPosition->m_position);
		}		

		return Result();
	}

}