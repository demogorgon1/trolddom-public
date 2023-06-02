#include "../Pcheader.h"

#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/ThreatSource.h>

#include <tpublic/DirectEffects/Heal.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Helpers.h>
#include <tpublic/IResourceChangeQueue.h>
#include <tpublic/IThreatEventQueue.h>
#include <tpublic/Resource.h>

namespace tpublic::DirectEffects
{

	void
	Heal::FromSource(
		const Parser::Node*		aSource) 
	{
		aSource->ForEachChild([&](
			const Parser::Node*	aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if(aChild->m_name == "base")
				{
					if(aChild->m_type == Parser::Node::TYPE_ARRAY)
					{
						if(aChild->m_children.size() == 1)
						{
							m_baseMin = aChild->m_children[0]->GetUInt32();
							m_baseMax = m_baseMin;
						}
						else if (aChild->m_children.size() == 2)
						{
							m_baseMin = aChild->m_children[0]->GetUInt32();
							m_baseMax = aChild->m_children[1]->GetUInt32();
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid heal base definition.", aChild->m_name.c_str());
						}
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "Not a valid heal base.", aChild->m_name.c_str());
					}
				}
				else
				{
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				}
			}
		});
	}

	void	
	Heal::ToStream(
		IWriter*				aStream) const 
	{
		ToStreamBase(aStream);
		aStream->WriteUInt(m_baseMin);
		aStream->WriteUInt(m_baseMax);
	}
			
	bool	
	Heal::FromStream(
		IReader*				aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		if (!aStream->ReadUInt(m_baseMin))
			return false;
		if (!aStream->ReadUInt(m_baseMax))
			return false;
		return true;
	}

	void
	Heal::Resolve(
		uint32_t					/*aTick*/,
		std::mt19937&				aRandom,
		const Manifest*				/*aManifest*/,
		CombatEvent::Id				aId,
		uint32_t					aAbilityId,
		const EntityInstance*		aSource,
		EntityInstance*				aTarget,
		IResourceChangeQueue*		aResourceChangeQueue,
		IAuraEventQueue*			/*aAuraEventQueue*/,
		IThreatEventQueue*			aThreatEventQueue) 
	{
		const Components::CombatPrivate* sourceCombatPrivate = aSource->GetComponent<Components::CombatPrivate>();
		Components::CombatPublic* targetCombatPublic = aTarget->GetComponent<Components::CombatPublic>();

		if(sourceCombatPrivate == NULL || targetCombatPublic == NULL)
			return;

		uint32_t heal = Helpers::RandomInRange(aRandom, m_baseMin, m_baseMax);
				
		CombatEvent::Id result = aId;

		if(m_flags & DirectEffect::FLAG_CAN_BE_CRITICAL && aId == CombatEvent::ID_HIT)
		{
			float chance = (float)sourceCombatPrivate->m_magicalCriticalStrikeChance / (float)UINT32_MAX;

			if(Helpers::RandomFloat(aRandom) < chance)
			{
				heal = (heal * 3) / 2;
				result = CombatEvent::ID_CRITICAL;
			}
		}

		size_t healthResourceIndex;
		if(targetCombatPublic->GetResourceIndex(Resource::ID_HEALTH, healthResourceIndex))
		{
			aResourceChangeQueue->AddResourceChange(
				result,
				aAbilityId,
				aSource->GetEntityInstanceId(),
				aTarget->GetEntityInstanceId(),
				targetCombatPublic,
				healthResourceIndex,
				(int32_t)heal);

			// Anyone on the target's threat target list should gain threat from this
			const Components::ThreatSource* targetThreatSource = aTarget->GetComponent<Components::ThreatSource>();

			if(targetThreatSource != NULL)
			{
				int32_t threat = (int32_t)heal;
				if(result == CombatEvent::ID_CRITICAL)
					threat = (threat * 3) / 2;

				for(std::unordered_map<uint32_t, uint32_t>::const_iterator i = targetThreatSource->m_targets.cbegin(); i != targetThreatSource->m_targets.cend(); i++)
				{
					aThreatEventQueue->AddThreatEvent(aSource->GetEntityInstanceId(), i->first, threat);
				}
			}
		}
	}

}