#pragma once

#include "../Components/CombatPrivate.h"
#include "../Components/CombatPublic.h"

#include "../EffectBase.h"
#include "../Helpers.h"
#include "../ICombatResultQueue.h"
#include "../Resource.h"

namespace kpublic
{

	namespace Effects
	{

		struct Damage
			: public EffectBase
		{
			static const Effect::Id ID = Effect::ID_DAMAGE;

			Damage()
			{

			}

			virtual 
			~Damage()
			{

			}

			// EffectBase implementation
			void
			FromSource(
				const Parser::Node*		aSource) override
			{
				aSource->ForEachChild([&](
					const Parser::Node*	aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if(aChild->m_name == "damage_type")
						{
							m_damageType = Effect::StringToDamageType(aChild->GetIdentifier());
						}
						else if(aChild->m_name == "base")
						{
							if(aChild->m_type == Parser::Node::TYPE_ARRAY)
							{
								m_damageBase = Effect::DAMAGE_BASE_RANGE;

								if(aChild->m_children.size() == 1)
								{
									m_damageBaseRangeMin = aChild->m_children[0]->GetUInt32();
									m_damageBaseRangeMax = m_damageBaseRangeMin;
								}
								else if (aChild->m_children.size() == 2)
								{
									m_damageBaseRangeMin = aChild->m_children[1]->GetUInt32();
									m_damageBaseRangeMax = aChild->m_children[2]->GetUInt32();
								}
								else
								{
									KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid damage base definition.", aChild->m_name.c_str());
								}
							}
							else if(aChild->m_type == Parser::Node::TYPE_IDENTIFIER && aChild->m_value == "weapon")
							{
								m_damageBase = Effect::DAMAGE_BASE_WEAPON;
							}
							else
							{
								KP_VERIFY(false, aChild->m_debugInfo, "Not a valid damage base.", aChild->m_name.c_str());
							}
						}
						else
						{
							KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
						}
					}
				});
			}

			void	
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WritePOD(m_damageType);
				aStream->WritePOD(m_damageBase);

				if(m_damageBase == Effect::DAMAGE_BASE_RANGE)
				{
					aStream->WriteUInt(m_damageBaseRangeMin);
					aStream->WriteUInt(m_damageBaseRangeMax);
				}
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				if (!aStream->ReadPOD(m_damageType))
					return false;
				if (!aStream->ReadPOD(m_damageBase))
					return false;

				if (m_damageBase == Effect::DAMAGE_BASE_RANGE)
				{
					if (!aStream->ReadUInt(m_damageBaseRangeMin))
						return false;
					if (!aStream->ReadUInt(m_damageBaseRangeMax))
						return false;
				}

				return true;
			}

			void
			Resolve(
				std::mt19937&				aRandom,
				Components::CombatPublic*	/*aSourceCombatPublic*/,
				Components::CombatPrivate*	aSourceCombatPrivate,
				Components::CombatPublic*	aTargetCombatPublic,
				CombatEvent::Id				aId,
				ICombatResultQueue*			aCombatResultQueue) override
			{
				uint32_t damage = 0;
				
				switch(m_damageBase)
				{
				case Effect::DAMAGE_BASE_RANGE:		
					damage = Helpers::RandomInRange(aRandom, m_damageBaseRangeMin, m_damageBaseRangeMax); 
					break;

				case Effect::DAMAGE_BASE_WEAPON:
					damage = Helpers::RandomInRange(aRandom, aSourceCombatPrivate->m_weaponDamageRangeMin, aSourceCombatPrivate->m_weaponDamageRangeMax);
					break;

				default:
					break;
				}

				CombatEvent::Id result = aId;

				if(m_flags & Effect::FLAG_CAN_BE_CRITICAL && aId == CombatEvent::ID_HIT)
				{
					float chance = 0.0f;

					if(m_flags & Effect::FLAG_IS_MAGICAL)
						chance = (float)aSourceCombatPrivate->m_magicalCriticalStrikeChance / (float)UINT32_MAX;
					else
						chance = (float)aSourceCombatPrivate->m_physicalCriticalStrikeChance / (float)UINT32_MAX;

					if(Helpers::RandomFloat(aRandom) < chance)
					{
						damage = (damage * 3) / 2;

						result = CombatEvent::ID_CRITICAL_STRIKE;
					}
				}

				size_t healthResourceIndex;
				if(aTargetCombatPublic->GetResourceIndex(Resource::ID_HEALTH, healthResourceIndex))
					aCombatResultQueue->AddResourceChange(aTargetCombatPublic, healthResourceIndex, -(int32_t)damage);
			}

			// Public data
			Effect::DamageType	m_damageType = Effect::DAMAGE_TYPE_PHYSICAL;
			Effect::DamageBase	m_damageBase = Effect::DAMAGE_BASE_RANGE;

			uint32_t			m_damageBaseRangeMin = 0;
			uint32_t			m_damageBaseRangeMax = 0;
		};

	}

}