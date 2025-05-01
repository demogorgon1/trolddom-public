#include "../Pcheader.h"

#include <tpublic/Data/Ability.h>

#include <tpublic/DirectEffects/LearnProfessionAbility.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/Manifest.h>

namespace tpublic::DirectEffects
{

	void
	LearnProfessionAbility::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if (aChild->m_name == "profession")
					m_professionId = aChild->GetId(DataType::ID_PROFESSION);
				else if (aChild->m_name == "ability")
					m_abilityId = aChild->GetId(DataType::ID_ABILITY);
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
			}
		});
	}

	void
	LearnProfessionAbility::ToStream(
		IWriter*						aStream) const 
	{	
		ToStreamBase(aStream);

		aStream->WriteUInt(m_professionId);
		aStream->WriteUInt(m_abilityId);
	}

	bool
	LearnProfessionAbility::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;

		if (!aStream->ReadUInt(m_professionId))
			return false;
		if (!aStream->ReadUInt(m_abilityId))
			return false;
		return true;
	}

	DirectEffectBase::Result
	LearnProfessionAbility::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					/*aRandom*/,
		const Manifest*					/*aManifest*/,
		CombatEvent::Id					/*aId*/,
		uint32_t						/*aAbilityId*/,
		const SourceEntityInstance&		/*aSourceEntityInstance*/,
		EntityInstance*					/*aSource*/,
		EntityInstance*					aTarget,
		const Vec2&						/*aAOETarget*/,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			/*aCombatResultQueue*/,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					aEventQueue,
		const IWorldView*				/*aWorldView*/) 
	{				
		aEventQueue->EventQueueLearnProfessionAbility(aTarget->GetEntityInstanceId(), m_professionId, m_abilityId);
		return Result();	
	}

	uint32_t		
	LearnProfessionAbility::GetToolTipItemId(
		const Manifest*					aManifest) const
	{
		const Data::Ability* ability = aManifest->GetById<Data::Ability>(m_abilityId);
		if(!ability->m_produceItems)
			return 0;

		if(ability->m_produceItems->m_items.size() != 1)
			return 0;

		const Data::Ability::Item& item = ability->m_produceItems->m_items[0];
		if(item.m_quantity != 1)
			return 0;

		return item.m_itemId;
	}

}