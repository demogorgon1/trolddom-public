#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>

#include <tpublic/DirectEffects/Damage.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Helpers.h>
#include <tpublic/IResourceChangeQueue.h>
#include <tpublic/IThreatEventQueue.h>

namespace tpublic::DirectEffects
{

	void
	Damage::FromSource(
		const Parser::Node*		aSource)
	{
		aSource->ForEachChild([&](
			const Parser::Node*	aChild)
		{
			if(!FromSourceBase(aChild))
			{
				if(aChild->m_name == "damage_type")
				{
					m_damageType = DirectEffect::StringToDamageType(aChild->GetIdentifier());
				}
				else if(aChild->m_name == "base")
				{
					if(aChild->m_type == Parser::Node::TYPE_ARRAY)
					{
						m_damageBase = DirectEffect::DAMAGE_BASE_RANGE;

						if(aChild->m_children.size() == 1)
						{
							m_damageBaseRangeMin = aChild->m_children[0]->GetUInt32();
							m_damageBaseRangeMax = m_damageBaseRangeMin;
						}
						else if (aChild->m_children.size() == 2)
						{
							m_damageBaseRangeMin = aChild->m_children[0]->GetUInt32();
							m_damageBaseRangeMax = aChild->m_children[1]->GetUInt32();
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid damage base definition.", aChild->m_name.c_str());
						}
					}
					else if(aChild->m_type == Parser::Node::TYPE_IDENTIFIER && aChild->m_value == "weapon")
					{
						m_damageBase = DirectEffect::DAMAGE_BASE_WEAPON;
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "Not a valid damage base.", aChild->m_name.c_str());
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
	Damage::ToStream(
		IWriter*				aStream) const
	{
		ToStreamBase(aStream);
		aStream->WritePOD(m_damageType);
		aStream->WritePOD(m_damageBase);

		if(m_damageBase == DirectEffect::DAMAGE_BASE_RANGE)
		{
			aStream->WriteUInt(m_damageBaseRangeMin);
			aStream->WriteUInt(m_damageBaseRangeMax);
		}
	}
			
	bool	
	Damage::FromStream(
		IReader*				aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		if (!aStream->ReadPOD(m_damageType))
			return false;
		if (!aStream->ReadPOD(m_damageBase))
			return false;

		if (m_damageBase == DirectEffect::DAMAGE_BASE_RANGE)
		{
			if (!aStream->ReadUInt(m_damageBaseRangeMin))
				return false;
			if (!aStream->ReadUInt(m_damageBaseRangeMax))
				return false;
		}

		return true;
	}

	void
	Damage::Resolve(
		int32_t						/*aTick*/,
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
		const Components::Auras* targetAuras = aTarget->GetComponent<Components::Auras>();

		if(targetCombatPublic == NULL)
			return;

		uint32_t damage = 0;

		switch(m_damageBase)
		{
		case DirectEffect::DAMAGE_BASE_RANGE:		
			damage = Helpers::RandomInRange(aRandom, m_damageBaseRangeMin, m_damageBaseRangeMax); 
			break;

		case DirectEffect::DAMAGE_BASE_WEAPON:
			TP_CHECK(sourceCombatPrivate != NULL, "No weapon damage available.");
			damage = Helpers::RandomInRange(aRandom, sourceCombatPrivate->m_weaponDamageRangeMin, sourceCombatPrivate->m_weaponDamageRangeMax);
			break;

		default:
			break;
		}

		CombatEvent::Id result = aId;

		if(m_flags & DirectEffect::FLAG_CAN_BE_CRITICAL && aId == CombatEvent::ID_HIT && sourceCombatPrivate != NULL)
		{
			float chance = 0.0f;

			if(m_flags & DirectEffect::FLAG_IS_MAGICAL)
				chance = (float)sourceCombatPrivate->m_magicalCriticalStrikeChance / (float)UINT32_MAX;
			else
				chance = (float)sourceCombatPrivate->m_physicalCriticalStrikeChance / (float)UINT32_MAX;

			if(Helpers::RandomFloat(aRandom) < chance)
			{
				damage = (damage * 3) / 2;

				result = CombatEvent::ID_CRITICAL;
			}
		}

		size_t healthResourceIndex;
		if(targetCombatPublic->GetResourceIndex(Resource::ID_HEALTH, healthResourceIndex))
		{
			if(targetAuras != NULL)
				damage = targetAuras->FilterDamageInput(m_damageType, damage);

			aResourceChangeQueue->AddResourceChange(
				result,
				aAbilityId,
				aSource->GetEntityInstanceId(),
				aTarget->GetEntityInstanceId(),
				targetCombatPublic,
				healthResourceIndex,
				-(int32_t)damage);

			int32_t threat = (int32_t)damage;
			if(result == CombatEvent::ID_CRITICAL)
				threat = (threat * 3) / 2;

			if(aTarget->GetEntityId() != 0) // Not a player
				aThreatEventQueue->AddThreatEvent(aSource->GetEntityInstanceId(), aTarget->GetEntityInstanceId(), threat);
		}
	}


}