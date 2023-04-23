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

		struct Heal
			: public EffectBase
		{
			static const Effect::Id ID = Effect::ID_HEAL;

			Heal()
			{

			}

			virtual 
			~Heal()
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
									KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid heal base definition.", aChild->m_name.c_str());
								}
							}
							else
							{
								KP_VERIFY(false, aChild->m_debugInfo, "Not a valid heal base.", aChild->m_name.c_str());
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
				aStream->WriteUInt(m_baseMin);
				aStream->WriteUInt(m_baseMax);
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
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
			Resolve(
				std::mt19937&				aRandom,
				Components::CombatPublic*	/*aSourceCombatPublic*/,
				Components::CombatPrivate*	aSourceCombatPrivate,
				Components::CombatPublic*	aTargetCombatPublic,
				CombatEvent::Id				aId,
				uint32_t					aAbilityId,
				uint32_t					aSourceEntityInstanceId,
				uint32_t					aTargetEntityInstanceId,
				ICombatResultQueue*			aCombatResultQueue) override
			{
				uint32_t heal = Helpers::RandomInRange(aRandom, m_baseMin, m_baseMax);
				
				CombatEvent::Id result = aId;

				if(m_flags & Effect::FLAG_CAN_BE_CRITICAL && aId == CombatEvent::ID_HIT)
				{
					float chance = (float)aSourceCombatPrivate->m_magicalCriticalStrikeChance / (float)UINT32_MAX;

					if(Helpers::RandomFloat(aRandom) < chance)
					{
						heal = (heal * 3) / 2;
						result = CombatEvent::ID_CRITICAL;
					}
				}

				size_t healthResourceIndex;
				if(aTargetCombatPublic->GetResourceIndex(Resource::ID_HEALTH, healthResourceIndex))
				{
					aCombatResultQueue->AddResourceChange(
						result,
						aAbilityId,
						aSourceEntityInstanceId,
						aTargetEntityInstanceId,
						aTargetCombatPublic, 
						healthResourceIndex,
						(int32_t)heal);

					//int32_t threat = (int32_t)damage;
					//if(result == CombatEvent::ID_CRITICAL)
					//	threat = (threat * 3) / 2;

					//aCombatResultQueue->AddThreatChange(aSourceEntityInstanceId, aTargetEntityInstanceId, threat);
				}
			}

			// Public data
			uint32_t			m_baseMin = 0;
			uint32_t			m_baseMax = 0;
		};

	}

}